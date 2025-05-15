# 注释一遍 CMakeLists.txt 然后找到入口文件 Calc.cpp , 从 Calc.cpp 开始看起

```cpp
static llvm::cl::opt<std::string>
    Input(llvm::cl::Positional,
          llvm::cl::desc("<input expression>"),
          llvm::cl::init(""));
```

- **`llvm::cl::opt`** 是 LLVM 提供的一个用于定义命令行选项的类模板。它允许你定义一个命令行参数，并指定其类型、位置、描述和默认值。
- **`llvm::cl::Positional`**：表示这是一个位置参数，即它不是以 `--option` 或 `-option` 形式出现的参数，而是直接跟在程序名后面的参数。例如，在命令行中 `calc "1 + 2"`，`"1 + 2"` 就是一个位置参数。
- **`llvm::cl::desc`**：用于描述该参数的用途，这在帮助信息中显示。例如，当用户运行 `calc --help` 时，会显示 `<input expression>` 的描述。
- **`llvm::cl::init`**：用于指定该参数的默认值。在这里，`Input` 的默认值为空字符串 `""`。

```cpp
llvm::InitLLVM X(argc, argv);
```

这段代码初始化 LLVM 的运行时环境，确保 LLVM 的各种功能可以正确运行。这是使用 LLVM 的标准做法，通常在程序的入口点调用。

```cpp
llvm::cl::ParseCommandLineOptions(
    argc, argv, "calc - the expression compiler\n");
```

- **`llvm::cl::ParseCommandLineOptions`** 是 LLVM 提供的一个函数，用于解析命令行选项。它会根据之前定义的命令行选项（如 `Input`）解析用户输入的命令行参数，并将解析结果存储到相应的变量中。
- **`argc` 和 `argv`**：这两个参数是从 `main` 函数传递过来的，分别表示命令行参数的数量和参数的值。
- **`"calc - the expression compiler\n"`**：这是一个字符串，用于在帮助信息中显示程序的描述。当用户运行 `calc --help` 时，会显示这个字符串。

```cpp
Lexer Lex(Input);
Parser Parser(Lex);
AST *Tree = Parser.parse();
if (!Tree || Parser.hasError()) {
    llvm::errs() << "Syntax errors occured\n";
    return 1;
}
Sema Semantic;
if (Semantic.semantic(Tree)) {
    llvm::errs() << "Semantic errors occured\n";
    return 1;
}
CodeGen CodeGenerator;
CodeGenerator.compile(Tree);
```

- **`Lexer Lex(Input)`** 词法分析, token 化输入的字符串
- **`Parser Parser(Lex)`** 语法分析, 将 tokens 解析成表达式。以 ast 的形式表示。
- **`Semantic.semantic(Tree)`** 简单的语义分析, 遍历 ast 确保树的正确性。
- **`CodeGenerator.compile(Tree)`** 编译为 LLVM ir 。

## Lexer 类
`Lexer` 提供了两个方法一是 `next`: 双指针识别字符串，转变为 “token 串”, 二是 `formToken`: 构造 token。

## Parser 类
核心方法为 `AST *Parser::parse()` 代表的解析函数, 文法比较少, 采用了递归下降的解析方式。
```bash
calc : ("with" ident ("," ident)* ":")? expr ;
expr : term (( "+" | "-" ) term)* ;
term : factor (( "*" | "/") factor)* ;
factor : ident | number | "(" expr ")" ;
ident : ([a-zA-Z])+ ;
number : ([0-9])+ ;
```
照着作者给的文法一个函数一个函数写就好了, 值得一提的是要是自定义文法使用这种 LL(1) + 递归下降 的接卸方式, 需要求 `first` 集合与 `fellow` 集合, 确保文法正确。

## Sema 类
核心方法为 `bool Sema::semantic(AST *Tree)` 用访问者模式以便配合多态。
```cpp
bool Sema::semantic(AST *Tree) {
  if (!Tree)
    return false;
  DeclCheck Check;
  Tree->accept(Check);
  return Check.hasError();
}
```
其中 DeclCheck 就是语义分析的访问者。

在 `AST *Parser::parse()` 中解析出的 ast 的节点类型在本例中只包含三种: `Factor` `BinaryOp` `WithDecl` 所以只需要实现这三个的 `visit` 方法就行了。

## CodeGen 类
和 `Sema` 类相似, 都是访问者模式去遍历 ast , 核心方法为 `void CodeGen::compile(AST *Tree)` 将 ast 翻译成 LLVM ir, 其中 `ToIRVisitor` 类是访问者, 因为有比较多的 LLVM 方法, 其实就是套路, 所以我直接贴一份完整的注释代码。

```cpp
#include "CodeGen.h"  // 包含代码生成模块的头文件
#include "llvm/ADT/StringMap.h"  // 包含 LLVM 的字符串映射表
#include "llvm/IR/IRBuilder.h"  // 包含 LLVM 的 IR 构建器
#include "llvm/IR/LLVMContext.h"  // 包含 LLVM 的上下文环境
#include "llvm/Support/raw_ostream.h"  // 包含 LLVM 的原始输出流

using namespace llvm;  // 使用 LLVM 命名空间

namespace {
// 定义一个匿名命名空间，其中包含代码生成的实现细节

class ToIRVisitor : public ASTVisitor {  // 定义一个继承自 ASTVisitor 的类
  Module *M;  // LLVM 模块指针
  IRBuilder<> Builder;  // LLVM IR 构建器
  Type *VoidTy;  // void 类型
  Type *Int32Ty;  // 32位整数类型
  PointerType *PtrTy;  // 指针类型
  Constant *Int32Zero;  // 32位整数常量 0

  Value *V;  // 当前操作的值
  StringMap<Value *> nameMap;  // 用于存储变量名到值的映射

public:
  // 构造函数，初始化 LLVM 模块和 IR 构建器
  ToIRVisitor(Module *M) : M(M), Builder(M->getContext()) {
    VoidTy = Type::getVoidTy(M->getContext());  // 获取 void 类型
    Int32Ty = Type::getInt32Ty(M->getContext());  // 获取 32位整数类型
    PtrTy = PointerType::getUnqual(M->getContext());  // 获取未限定的指针类型
    Int32Zero = ConstantInt::get(Int32Ty, 0, true);  // 创建常量 0
  }

  // 主函数，用于生成 LLVM IR
  void run(AST *Tree) {
    // 定义 main 函数的类型
    FunctionType *MainFty = FunctionType::get(
        Int32Ty, {Int32Ty, PtrTy}, false);
    // 创建 main 函数
    Function *MainFn = Function::Create(
        MainFty, GlobalValue::ExternalLinkage, "main", M);
    // 创建一个基本块
    BasicBlock *BB = BasicBlock::Create(M->getContext(),
                                        "entry", MainFn);
    // 设置 IR 构建器的插入点
    Builder.SetInsertPoint(BB);

    // 遍历 AST 树
    Tree->accept(*this);

    // 定义 calc_write 函数的类型
    FunctionType *CalcWriteFnTy =
        FunctionType::get(VoidTy, {Int32Ty}, false);
    // 创建 calc_write 函数
    Function *CalcWriteFn = Function::Create(
        CalcWriteFnTy, GlobalValue::ExternalLinkage,
        "calc_write", M);
    // 调用 calc_write 函数
    Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {V});

    // 返回 0
    Builder.CreateRet(Int32Zero);
  }

  // 虚拟函数，用于访问 Factor 节点
  virtual void visit(Factor &Node) override {
    if (Node.getKind() == Factor::Ident) {  // 如果是标识符
      V = nameMap[Node.getVal()];  // 从映射表中获取值
    } else {  // 如果是整数
      int intval;
      Node.getVal().getAsInteger(10, intval);  // 获取整数值
      V = ConstantInt::get(Int32Ty, intval, true);  // 创建常量
    }
  };

  // 虚拟函数，用于访问 BinaryOp 节点
  virtual void visit(BinaryOp &Node) override {
    // 访问左子节点
    Node.getLeft()->accept(*this);
    Value *Left = V;
    // 访问右子节点
    Node.getRight()->accept(*this);
    Value *Right = V;
    // 根据操作符生成相应的 LLVM IR
    switch (Node.getOperator()) {
    case BinaryOp::Plus:
      V = Builder.CreateNSWAdd(Left, Right);  // 加法
      break;
    case BinaryOp::Minus:
      V = Builder.CreateNSWSub(Left, Right);  // 减法
      break;
    case BinaryOp::Mul:
      V = Builder.CreateNSWMul(Left, Right);  // 乘法
      break;
    case BinaryOp::Div:
      V = Builder.CreateSDiv(Left, Right);  // 除法
      break;
    }
  };

  // 虚拟函数，用于访问 WithDecl 节点
  virtual void visit(WithDecl &Node) override {
    // 定义 calc_read 函数的类型
    FunctionType *ReadFty =
        FunctionType::get(Int32Ty, {PtrTy}, false);
    // 创建 calc_read 函数
    Function *ReadFn = Function::Create(
        ReadFty, GlobalValue::ExternalLinkage, "calc_read",
        M);
    // 遍历 WithDecl 节点中的变量声明
    for (auto I = Node.begin(), E = Node.end(); I != E;
         ++I) {
      StringRef Var = *I;

      // 创建字符串常量
      Constant *StrText = ConstantDataArray::getString(
          M->getContext(), Var);
      // 创建全局变量
      GlobalVariable *Str = new GlobalVariable(
          *M, StrText->getType(),
          /*isConstant=*/true, GlobalValue::PrivateLinkage,
          StrText, Twine(Var).concat(".str"));
      // 调用 calc_read 函数
      CallInst *Call =
          Builder.CreateCall(ReadFty, ReadFn, {Str});

      // 将变量名映射到调用结果
      nameMap[Var] = Call;
    }

    // 访问表达式
    Node.getExpr()->accept(*this);
  };
};
} // namespace

// CodeGen 类的 compile 方法
void CodeGen::compile(AST *Tree) {
  LLVMContext Ctx;  // 创建 LLVM 上下文
  Module *M = new Module("calc.expr", Ctx);  // 创建 LLVM 模块
  ToIRVisitor ToIR(M);  // 创建 ToIRVisitor 对象
  ToIR.run(Tree);  // 调用 run 方法生成 LLVM IR
  M->print(outs(), nullptr);  // 将生成的 LLVM IR 输出到标准输出
}
```

## 总结
整体来说是比较简单的一个解释器，一看就懂，但麻雀虽小五脏六腑俱全，作为开篇来说是一个很好的例子。