# LLVM 添加优化管道套路

## PassBuilder 初始化与插件加载
PassBuilder 实例化, 遍历 PassPlugins 列表，使用 `PassPlugin::Load()` 加载用户指定的插件库，并通过 `registerPassBuilderCallbacks()` 方法注册插件中的自定义 Pass 到 PassBuilder
```cpp
// Create the optimization pipeline
PassBuilder PB(TM);

// Load requested pass plugins and let them register
// pass builder callbacks
for (auto &PluginFN : PassPlugins) {
    auto PassPlugin = PassPlugin::Load(PluginFN);
    if (!PassPlugin) {
        WithColor::error(errs(), Argv0)
            << "Failed to load passes from '" << PluginFN
            << "'. Request ignored.\n";
        continue;
}

PassPlugin->registerPassBuilderCallbacks(PB);
}
```

## 分析管理器注册与配置
创建四类分析管理器, 通过 `PB.register*Analyses()` 方法注册默认分析 Pass 并通过 `crossRegisterProxies()` 实现跨层级数据共享
```cpp
LoopAnalysisManager LAM;
FunctionAnalysisManager FAM;
CGSCCAnalysisManager CGAM;
ModuleAnalysisManager MAM;

// Register the AA manager first so that our version
// is the one used.
FAM.registerPass(
    [&] { return PB.buildDefaultAAPipeline(); });

// Register all the basic analyses with the managers.
PB.registerModuleAnalyses(MAM);
PB.registerCGSCCAnalyses(CGAM);
PB.registerFunctionAnalyses(FAM);
PB.registerLoopAnalyses(LAM);
PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
```

## 解析传递管道
如果用户指定了自定义传递管道（PassPipeline），则解析并构建传递管道:
```cpp
if (!PassPipeline.empty()) {
    if (auto Err = PB.parsePassPipeline(
            MPM, PassPipeline)) {
        WithColor::error(errs(), Argv0)
            << toString(std::move(Err)) << "\n";
        return false;
}
}
```
如果没有指定，则根据优化级别（OptLevel）选择默认的传递管道:
```cpp
StringRef DefaultPass;
switch (OptLevel) {
    case 0: DefaultPass = "default<O0>"; break;
    case 1: DefaultPass = "default<O1>"; break;
    case 2: DefaultPass = "default<O2>"; break;
    case 3: DefaultPass = "default<O3>"; break;
    case -1: DefaultPass = "default<Os>"; break;
    case -2: DefaultPass = "default<Oz>"; break;
}
if (auto Err = PB.parsePassPipeline(
        MPM, DefaultPass)) {
    WithColor::error(errs(), Argv0)
        << toString(std::move(Err)) << "\n";
    return false;
}
```

## 运行优化管道
```cpp
MPM.run(*M, MAM);
```
## 代码生成
使用 TargetMachine 和 legacy::PassManager 生成目标代码:
```cpp
legacy::PassManager CodeGenPM;
    CodeGenPM.add(createTargetTransformInfoWrapperPass(
        TM->getTargetIRAnalysis()));
    if (FileType == CGFT_AssemblyFile && EmitLLVM) {
    CodeGenPM.add(createPrintModulePass(Out->os()));
    } else {
    if (TM->addPassesToEmitFile(CodeGenPM, Out->os(),
                                nullptr, FileType)) {
        WithColor::error()
            << "No support for file type\n";
        return false;
    }
}

CodeGenPM.run(*M);
```