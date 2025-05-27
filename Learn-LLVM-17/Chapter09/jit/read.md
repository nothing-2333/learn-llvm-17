## 好吧, 这个比较简单先看 JIT.cpp 吧, 看起来像是实现主要逻辑的地方
有的时候感觉看代码像是在看一个故事, 需要一幕一幕的看, 否则你联系不起来。从这开始吧:
```cpp
int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv); // 初始化 LLVM 环境

  // 初始化目标相关组件
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  // 解析命令行选项
  cl::ParseCommandLineOptions(argc, argv, "jitty\n");

  // 创建 LLVM 上下文
  auto Ctx = std::make_unique<LLVMContext>();

  // 加载模块
  std::unique_ptr<Module> M =
      loadModule(InputFile, *Ctx, argv[0]);

  // 创建错误处理对象
  ExitOnError ExitOnErr(std::string(argv[0]) + ": ");
  // 调用 JIT 主逻辑
  ExitOnErr(jitmain(std::move(M), std::move(Ctx), argc, argv));

  return 0; // 返回成功
}
```
没什么特殊的, 进入 jitmain:
```cpp
Error jitmain(std::unique_ptr<Module> M, // LLVM 模块
              std::unique_ptr<LLVMContext> Ctx, // LLVM 上下文
              int argc, char *argv[]) { // 程序参数
  auto JIT = JIT::create(); // 创建 JIT 实例
  if (!JIT)
    return JIT.takeError();

  // 将模块添加到 JIT 中
  if (auto Err = (*JIT)->addIRModule(
          orc::ThreadSafeModule(std::move(M), // 将模块和上下文传递给 JIT
                                std::move(Ctx))))
    return Err; // 如果添加失败，返回错误

  // 查找名为 "main" 的符号
  auto MainSym = (*JIT)->lookup("main");
  if (!MainSym)
    return MainSym.takeError(); // 如果未找到，返回错误

  // 获取 "main" 函数的地址
  llvm::orc::ExecutorAddr MainExecutorAddr = MainSym->getAddress();
  auto *Main = MainExecutorAddr.toPtr<int(int, char**)>(); // 转换为函数指针

  // 调用 "main" 函数
  (void)Main(argc, argv);
  return Error::success(); // 返回成功
}
```
总的来说是利用 llvm 提供的 api 运行 ir。
