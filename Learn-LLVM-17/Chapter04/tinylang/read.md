## 重点摘录

### 基本块
基本块是一段​​顺序执行​​的指令序列，内部无分支或跳转。仅能通过基本块的第一条指令（通常为标签）进入，其他位置无法跳转至此块中间。最后一条指令必须是​​终结指令​​（如跳转 br、返回 ret 等），控制流仅能通过此指令离开。

### SSA
采用静态单一赋值(SSA)形式。代码使用无限量的虚拟寄存器，但每个寄存器只写入一次。内存访问通过 load 和 store 指令处理，这些指令本身不受SSA单次赋值的限制。

### phi
在循环中处理变量更新时，静态单赋值（SSA）形式的约束会导致寄存器只能被赋值一次，而循环中的变量需要多次更新。phi 指令​​正是为了解决这一问题而设计的，Phi指令​​允许在基本块的入口处动态选择变量的值，具体值取决于执行到该块时是从哪个前驱基本块跳转而来的。

```llvm
%var = phi <type> [ <value1>, <label1> ], [ <value2>, <label2> ], ...
```

示例：`gcd` 循环
```llvm
define i32 @gcd(i32 %a, i32 %b) {
entry:
  %cond = icmp eq i32 %b, 0
  br i1 %cond, label %exit, label %loop

loop:
  ; 定义Phi节点，根据前驱块选择值
  %a_curr = phi i32 [ %b, %entry ], [ %b_prev, %loop ]
  %b_curr = phi i32 [ %a, %entry ], [ %rem, %loop ]
  ; 计算余数
  %rem = srem i32 %a_curr, %b_curr
  ; 准备下一次迭代的值
  %b_prev = add i32 %b_curr, 0  ; 仅示例，实际可能直接传递 %b_curr
  ; 检查循环条件
  %cond_loop = icmp ne i32 %rem, 0
  br i1 %cond_loop, label %loop, label %exit

exit:
  %result = phi i32 [ %a, %entry ], [ %a_curr, %loop ]
  ret i32 %result
}
```

---
phi指令的执行逻辑
1. 入口基本块（entry）  
   • 当首次进入 `loop` 时，Phi指令的前驱块是 `entry`，因此：

     ◦ `%a_curr` 选择来自 `entry` 的值 `%b`（初始参数）。

     ◦ `%b_curr` 选择来自 `entry` 的值 `%a`（初始参数）。


2. 循环基本块（loop）  
   • 当从 `loop` 自身跳转回来时，Phi指令的前驱块是 `loop`，因此：

     ◦ `%a_curr` 选择来自 `loop` 的值 `%b_prev`（即上一次迭代的 `%b_curr`）。

     ◦ `%b_curr` 选择来自 `loop` 的值 `%rem`（即上一次的余数）。


3. 退出基本块（exit）  
   • 根据是从 `entry` 还是 `loop` 跳转而来，返回对应的结果。

---

使用phi指令有一些限制，必须是基本块的第一个指令。第一个基本块比较特殊:没有之前执行过的块，所以不能以phi指令开始。我们只触及了LLVM IR的皮毛，访问LLVM语言参考手册 https://llvm.org/docs/LangRef.html, 可了解更多细节。

## 源码阅读

### 在上一章的基础上增加, 简单扫一眼 `CMakeLists.txt` , 直接看 `tools/driver/Driver.cpp`
将其中一些新添的代码做注释, 可以简单扫一眼, 用的都是 llvm 提供的 api, 来做一些准备工作:
```cpp
// 定义命令行选项：输入文件
static llvm::cl::opt<std::string>
    InputFile(llvm::cl::Positional, // 位置参数
              llvm::cl::desc("<input-files>"), // 描述
              cl::init("-")); // 默认值为"-"

// 定义命令行选项：输出文件名
static llvm::cl::opt<std::string>
    OutputFilename("o", // 选项名称
                   llvm::cl::desc("Output filename"), // 描述
                   llvm::cl::value_desc("filename")); // 值的描述

// 定义命令行选项：目标三元组
static llvm::cl::opt<std::string> MTriple(
    "mtriple", // 选项名称
    llvm::cl::desc("Override target triple for module")); // 描述

// 定义命令行选项：是否生成LLVM IR代码
static llvm::cl::opt<bool> EmitLLVM(
    "emit-llvm", // 选项名称
    llvm::cl::desc("Emit IR code instead of assembler"), // 描述
    llvm::cl::init(false)); // 默认值为false

// 程序头部信息
static const char *Head = "tinylang - Tinylang compiler";

// 打印版本信息
void printVersion(llvm::raw_ostream &OS) {
  OS << Head << " " << getTinylangVersion() << "\n"; // 打印程序名称和版本号
  OS << "  Default target: " // 打印默认目标三元组
     << llvm::sys::getDefaultTargetTriple() << "\n";
  std::string CPU(llvm::sys::getHostCPUName()); // 获取宿主CPU名称
  OS << "  Host CPU: " << CPU << "\n"; // 打印宿主CPU名称
  OS << "\n";
  OS.flush(); // 刷新输出流
  llvm::TargetRegistry::printRegisteredTargetsForVersion( // 打印已注册的目标信息
      OS);
  exit(EXIT_SUCCESS); // 正常退出程序
}

// 创建目标机器
llvm::TargetMachine *createTargetMachine(const char *Argv0) {
  // 根据命令行选项或默认值创建目标三元组
  llvm::Triple Triple = llvm::Triple(
      !MTriple.empty()
          ? llvm::Triple::normalize(MTriple) // 如果指定了目标三元组，则使用它
          : llvm::sys::getDefaultTargetTriple()); // 否则使用默认目标三元组

  // 初始化目标选项
  llvm::TargetOptions TargetOptions =
      codegen::InitTargetOptionsFromCodeGenFlags(Triple);
  std::string CPUStr = codegen::getCPUStr(); // 获取目标CPU名称
  std::string FeatureStr = codegen::getFeaturesStr(); // 获取目标特性字符串

  std::string Error; // 用于存储错误信息
  const llvm::Target *Target =
      llvm::TargetRegistry::lookupTarget( // 查找目标机器
          codegen::getMArch(), Triple, Error);

  if (!Target) { // 如果查找失败
    llvm::WithColor::error(llvm::errs(), Argv0) << Error; // 打印错误信息
    return nullptr; // 返回空指针
  }

  // 创建目标机器
  llvm::TargetMachine *TM = Target->createTargetMachine(
      Triple.getTriple(), CPUStr, FeatureStr, TargetOptions,
      std::optional<llvm::Reloc::Model>(codegen::getRelocModel()));
  return TM;
}

bool emit(StringRef Argv0, llvm::Module *M, llvm::TargetMachine *TM,
          StringRef InputFilename) {
  // 获取代码生成文件类型
  CodeGenFileType FileType = codegen::getFileType();
  if (OutputFilename.empty()) { // 如果未指定输出文件名
    if (InputFilename == "-") { // 如果输入文件名为"-"
      OutputFilename = "-"; // 输出文件名也设为"-"
    } else {
      if (InputFilename.endswith(".mod")) // 如果输入文件以".mod"结尾
        OutputFilename = InputFilename.drop_back(4).str(); // 去掉后缀
      else
        OutputFilename = InputFilename.str(); // 否则直接使用输入文件名
      switch (FileType) { // 根据文件类型追加后缀
      case CGFT_AssemblyFile:
        OutputFilename.append(EmitLLVM ? ".ll" : ".s"); // 如果生成LLVM IR代码，则追加".ll"，否则追加".s"
        break;
      case CGFT_ObjectFile:
        OutputFilename.append(".o"); // 追加".o"
        break;
      case CGFT_Null:
        OutputFilename.append(".null"); // 追加".null"
        break;
      }
    }
  }

  // 打开输出文件
  std::error_code EC;
  sys::fs::OpenFlags OpenFlags = sys::fs::OF_None;
  if (FileType == CGFT_AssemblyFile)
    OpenFlags |= sys::fs::OF_TextWithCRLF; // 如果是汇编文件类型，则以文本模式打开
  auto Out = std::make_unique<llvm::ToolOutputFile>( // 创建工具输出文件
      OutputFilename, EC, OpenFlags);
  if (EC) { // 如果打开失败
    WithColor::error(llvm::errs(), Argv0) << EC.message() << '\n'; // 打印错误信息
    return false; // 返回false
  }

  legacy::PassManager PM; // 创建旧版Pass管理器
  if (FileType == CGFT_AssemblyFile && EmitLLVM) { // 如果生成LLVM IR代码
    PM.add(createPrintModulePass(Out->os())); // 添加模块打印Pass
  } else {
    if (TM->addPassesToEmitFile(PM, Out->os(), nullptr, FileType)) { // 添加目标机器的Pass
      WithColor::error(llvm::errs(), Argv0) << "No support for file type\n"; // 打印错误信息
      return false; // 返回false
    }
  }
  PM.run(*M); // 运行Pass管理器
  Out->keep(); // 保持输出文件
  return true; // 返回true
}

// 主函数
int main(int Argc, const char **Argv) {
  llvm::InitLLVM X(Argc, Argv); // 初始化LLVM

  // 初始化所有目标、目标MC、汇编打印机和解析器
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();

  // 设置版本信息打印函数
  llvm::cl::SetVersionPrinter(&printVersion);
  // 解析命令行选项
  llvm::cl::ParseCommandLineOptions(Argc, Argv, Head);

  // 检查是否需要打印CPU和特性帮助信息
  if (codegen::getMCPU() == "help" ||
      std::any_of(codegen::getMAttrs().begin(), codegen::getMAttrs().end(),
                  [](const std::string &a) {
                    return a == "help";
                  })) {
    auto Triple = llvm::Triple(LLVM_DEFAULT_TARGET_TRIPLE); // 获取默认目标三元组
    std::string ErrMsg;
    if (auto Target = llvm::TargetRegistry::lookupTarget( // 查找目标机器
            Triple.getTriple(), ErrMsg)) {
      llvm::errs() << "Targeting " << Target->getName() << ". "; // 打印目标名称
      // 打印目标的CPU和特性信息
      Target->createMCSubtargetInfo(
          Triple.getTriple(), codegen::getCPUStr(), codegen::getFeaturesStr());
    } else {
      llvm::errs() << ErrMsg << "\n"; // 打印错误信息
      exit(EXIT_FAILURE); // 退出程序
    }
    exit(EXIT_SUCCESS); // 正常退出程序
  }

  // 创建目标机器
  llvm::TargetMachine *TM = createTargetMachine(Argv[0]);
  if (!TM) // 如果创建失败
    exit(EXIT_FAILURE); // 退出程序

  // ......
}
```
然后把重点放在这段:
```cpp
llvm::SourceMgr SrcMgr;
DiagnosticsEngine Diags(SrcMgr);

// Tell SrcMgr about this buffer, which is what the
// parser will pick up.
SrcMgr.AddNewSourceBuffer(std::move(*FileOrErr),
                          llvm::SMLoc());

auto TheLexer = Lexer(SrcMgr, Diags);
auto TheSema = Sema(Diags);
auto TheParser = Parser(TheLexer, TheSema);
auto *Mod = TheParser.parse();
if (Mod && !Diags.numErrors()) {
  llvm::LLVMContext Ctx;
  if (CodeGenerator *CG =
          CodeGenerator::create(Ctx, TM)) {
    std::unique_ptr<llvm::Module> M =
        CG->run(Mod, InputFile);
    if (!emit(Argv[0], M.get(), TM, InputFile)) {
      llvm::WithColor::error(llvm::errs(), Argv[0])
          << "Error writing output\n";
    }
    delete CG;
  }
}
```
进行完语义分析后, 我们得到了一个合格的 ast: Mod, 然后交给 `CodeGenerator` 去生成 llvm ir, 再由 `emit` 输出, 在 `emit` 函数中拓展了一些其他的输出方式。
```bash
# 输出为 llvm ir
./tinylang --filetype=asm --emit-llvm examples/Gcd.mod
# 输出为 x64 汇编
./tinylang --filetype=asm examples/Gcd.mod
# 输出为 .o 文件
./tinylang --filetype=obj examples/Gcd.mod
```

### CodeGenerator 类
核心方法为 `CodeGenerator::run` 做了一些初始化的工作:
```cpp
// 创建 LLVM 模块
std::unique_ptr<llvm::Module> M =
    std::make_unique<llvm::Module>(FileName, Ctx); // 使用文件名和 LLVM 上下文初始化模块
M->setTargetTriple(TM->getTargetTriple().getTriple()); // 设置目标三元组
M->setDataLayout(TM->createDataLayout()); // 设置数据布局

// 创建代码生成模块并运行
CGModule CGM(M.get()); // 创建代码生成模块实例
CGM.run(Mod); // 运行代码生成模块，处理模块声明
```
### CGModule 类
重点还是在 `CGModule::run` 方法, 在 `CGModule` 类中有一些辅助方法: `CGModule::mangleName`名称修饰, `CGModule::convertType`类型转换。回到 `CGModule::run`, 整个函数比较短, 直接贴出注释代码:
```cpp
void CGModule::run(ModuleDeclaration *Mod) {
  for (auto *Decl : Mod->getDecls()) { // 遍历模块中的所有声明
    if (auto *Var = llvm::dyn_cast<VariableDeclaration>(Decl)) { // 如果是变量声明
      // 创建全局变量
      llvm::GlobalVariable *V = new llvm::GlobalVariable(
          *M, convertType(Var->getType()), // 使用变量声明的类型
          /*isConstant=*/false, // 不是常量
          llvm::GlobalValue::PrivateLinkage, // 私有链接
          nullptr, // 初始值为 nullptr
          mangleName(Var)); // 使用修饰后的名称
      Globals[Var] = V; // 将全局变量存储到 Globals 映射中
    } else if (auto *Proc = llvm::dyn_cast<ProcedureDeclaration>(Decl)) { // 如果是函数声明
      CGProcedure CGP(*this); // 创建 CGProcedure 实例
      CGP.run(Proc); // 对函数声明进行代码生成
    }
  }
}
```
### CGProcedure 类
可以说这个类是本章的灵魂, 还是从 `CGProcedure::run` 方法看起, 前面这块准备工作:
```cpp
Fty = createFunctionType(Proc); // 创建函数类型
Fn = createFunction(Proc, Fty); // 根据函数类型创建 LLVM 函数

// 创建入口基本块
llvm::BasicBlock *BB = llvm::BasicBlock::Create(
    CGM.getLLVMCtx(), "entry", Fn); // 在函数中创建一个名为 "entry" 的基本块
setCurr(BB); // 设置当前基本块为入口基本块
```
处理函数的参数和局部变量, tinylang 的局部变量写在开头:
```cpp
// 处理函数的形式参数
for (auto Pair : llvm::enumerate(Fn->args())) { // 遍历 LLVM 函数的所有参数
  llvm::Argument *Arg = &Pair.value(); // 获取当前参数
  FormalParameterDeclaration *FP =
      Proc->getFormalParams()[Pair.index()]; // 获取对应的形式参数声明
  // 为 VAR 参数创建映射（形式参数 -> LLVM 参数）
  FormalParams[FP] = Arg; // 将形式参数与 LLVM 参数关联起来
  writeLocalVariable(Curr, FP, Arg); // 将参数值写入当前基本块的局部变量定义表
}

// 处理过程中的变量声明
for (auto *D : Proc->getDecls()) { // 遍历过程中的所有声明
  if (auto *Var = llvm::dyn_cast<VariableDeclaration>(D)) { // 如果是变量声明
    llvm::Type *Ty = mapType(Var); // 将变量声明映射为 LLVM 类型
    if (Ty->isAggregateType()) { // 如果是聚合类型（如结构体或数组）
      // 为变量分配栈内存
      llvm::Value *Val = Builder.CreateAlloca(Ty); // 在栈上为变量分配内存
      writeLocalVariable(Curr, Var, Val); // 将分配的内存地址写入局部变量定义表
    }
  }
}
```
这是我们的重点, 为不同节点类型生成 ir:
```cpp
// 生成过程中的语句
auto Block = Proc->getStmts(); // 获取过程中的语句块
emit(Proc->getStmts()); // 递归处理语句块中的所有语句

// 检查是否需要添加默认的返回指令
if (!Curr->getTerminator()) { // 如果当前基本块没有终止指令
  Builder.CreateRetVoid(); // 添加一个空返回指令
}
sealBlock(Curr); // 封闭当前基本块，完成代码生成
```
进入 `emit` 函数, 这个比较容易看就不注释了:
```cpp
void CGProcedure::emit(const StmtList &Stmts) {
  for (auto *S : Stmts) {
    if (auto *Stmt = llvm::dyn_cast<AssignmentStatement>(S))
      emitStmt(Stmt);
    else if (auto *Stmt =
                 llvm::dyn_cast<ProcedureCallStatement>(S))
      emitStmt(Stmt);
    else if (auto *Stmt = llvm::dyn_cast<IfStatement>(S))
      emitStmt(Stmt);
    else if (auto *Stmt = llvm::dyn_cast<WhileStatement>(S))
      emitStmt(Stmt);
    else if (auto *Stmt =
                 llvm::dyn_cast<ReturnStatement>(S))
      emitStmt(Stmt);
    else
      llvm_unreachable("Unknown statement");
  }
}
```
就是利用 `emitStmt` 的重载去处理不同节点类型, 有点像前面见过的递归下降(我决定本质是一个东西, 就是递归去解析, 每个函数负责一个领域, 定义好出口, 这样递归下去会自己找到路)。基本块要自己判断, while、 if 类型的节点就会有很多基本快, 赋值就不会, 通过 `setCurr` 函数设置当前的基本块是哪个, 记录块间跳转关系, 也就是在构建控制流图CFG。

关于 `使用AST 编号生成SSA 格式的IR 代码` 为了将 ast 转成 SSA 格式, 定义了如下结构体:
```cpp
struct BasicBlockDef {
  // 将变量（或形式参数）映射到其定义的值。
  // 使用llvm::DenseMap存储变量声明（Decl）到LLVM值（llvm::Value）的映射。
  // llvm::TrackingVH<llvm::Value> 是一个智能指针，用于跟踪LLVM值的生命周期。
  llvm::DenseMap<Decl *, llvm::TrackingVH<llvm::Value>> Defs;

  // 存储尚未完成的phi指令的集合。
  // Phi指令通常用于表示控制流合并点的值，需要在所有前驱基本块确定后才能完成。
  // 这里将phi指令（llvm::PHINode）映射到对应的变量声明（Decl）。
  llvm::DenseMap<llvm::PHINode *, Decl *> IncompletePhis;

  // 标记基本块是否已封闭（sealed）。
  // 如果基本块已封闭，则表示不会再有新的前驱基本块添加到该基本块。
  // 这是一个位字段，占用1位。
  unsigned Sealed : 1;

  // 构造函数，初始化Sealed为0（表示基本块未封闭）。
  BasicBlockDef() : Sealed(0) {}
};
```
关于 SSA 这一块, 需要明确一点: ast 中一个变量可以对应很多值, 在 llvm ir 中一个变量对应一个值(严格来说不完全是这样, 遵循 SSA)。在生成 ir 时, 传入的都是值, 而不是变量, llvm 会为每个值生成编号也就是变量名, 我们只需要传入值就好, 为此我们维护了 Defs 记录`基本块-变量定义节点-值`的映射, 以方便获取到正确的值; 

在 llvm ir 中在获取变量的值的过程用到了递归检索 `readLocalVariable(PredBB, Decl)` 来解决有些变量的值在当前基本块用到了却在当前基本块找不到, 这时我们需要向前查找, 在遍历过程中有时存在尚未定义变量的值, 在去读值时, 如果在当前块找不到值时, 我们插入一条空 phi 指令 `llvm::PHINode *Phi = addEmptyPhi(BB, Decl);` 将其作为值, 同时记录尚未完成的 phi 指令 `CurrentDef[BB].IncompletePhis[Phi] = Decl;` 然后检索前驱填充 phi 指令, 如果发现 phi 是多余的, 及去除。

再来看看这个函数:
```cpp
void CGProcedure::writeVariable(llvm::BasicBlock *BB, Decl *D, llvm::Value *Val) {
  // 尝试将 Decl* 类型的 D 动态转换为 VariableDeclaration 类型
  if (auto *V = llvm::dyn_cast<VariableDeclaration>(D)) {
    // 检查变量声明所属的作用域
    if (V->getEnclosingDecl() == Proc) {
      // 如果变量声明属于当前过程（Proc），则调用 writeLocalVariable 函数
      // 将值 Val 写入局部变量
      writeLocalVariable(BB, D, Val);
    } else if (V->getEnclosingDecl() == CGM.getModuleDeclaration()) {
      // 如果变量声明属于模块作用域（全局变量），则使用 Builder.CreateStore
      // 将值 Val 存储到全局变量中
      Builder.CreateStore(Val, CGM.getGlobal(D));
    } else {
      // 如果变量声明属于嵌套过程的作用域，当前代码不支持嵌套过程
      // 因此报告致命错误
      llvm::report_fatal_error("Nested procedures not yet supported");
    }
  } 
  // 尝试将 Decl* 类型的 D 动态转换为 FormalParameterDeclaration 类型
  else if (auto *FP = llvm::dyn_cast<FormalParameterDeclaration>(D)) {
    // 检查形式参数是否是引用参数（VAR 参数）
    if (FP->isVar()) {
      // 如果是引用参数，使用 Builder.CreateStore 将值 Val 存储到
      // FormalParams 映射中对应的形式参数地址
      Builder.CreateStore(Val, FormalParams[FP]);
    } else {
      // 如果不是引用参数，调用 writeLocalVariable 函数将值 Val 写入局部变量
      writeLocalVariable(BB, D, Val);
    }
  } 
  // 如果 D 既不是 VariableDeclaration 也不是 FormalParameterDeclaration 类型
  else {
    // 报告不支持的声明类型错误
    llvm::report_fatal_error("Unsupported declaration");
  }
}
```
发现一些局部变量是调用 `writeLocalVariable` 函数写入的, 而 `writeLocalVariable` 函数只做了映射储存了值, 在用到是的时候拿出来, 而根本不会有对应的 llvm ir, 比如在代码中定义局部变量 `a = 1` 实际中根本不会有对应的 ir, 这是由于 llvm ir SSA 的性质导致的, 因为 llvm ir 变量只对应一个值, 所以像这样的赋值常量, 直接用就好了, 不需要再多定义一个变量, 类似常量传播。