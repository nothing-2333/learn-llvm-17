## 还是注释 CMakeLists.txt 找到入口文件 `tools/driver/Driver.cpp`
```cpp
llvm::SmallVector<const char *, 256> argv(argv_ + 1, argv_ + argc_);
```
argv_ + 1 表示跳过第一个参数，argv_ + argc_ 表示参数数组的末尾。这段代码将命令行参数（除了程序名）存储到 argv 中。

```cpp
llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> FileOrErr = llvm::MemoryBuffer::getFile(F);
```
- **`llvm::ErrorOr`**：
  - `llvm::ErrorOr` 是 LLVM 提供的一个模板类，用于表示可能出错的操作的结果。
  - 它可以存储一个成功的结果（如 `std::unique_ptr<llvm::MemoryBuffer>`）或一个错误代码（`std::error_code`）。
- **`llvm::MemoryBuffer::getFile`**：
  - 这是一个静态函数，尝试打开指定的文件并返回一个内存缓冲区。
  - 如果文件打开成功，返回一个包含文件内容的 `std::unique_ptr<llvm::MemoryBuffer>`。
  - 如果文件打开失败，返回一个错误代码。

```cpp
llvm::SourceMgr SrcMgr;
```
- **`llvm::SourceMgr`**：
  - `llvm::SourceMgr` 是 LLVM 提供的一个源代码管理器类，用于管理源代码文件。
  - 它负责跟踪源代码文件的缓冲区、文件名、行号等信息。

```cpp
SrcMgr.AddNewSourceBuffer(std::move(*FileOrErr), llvm::SMLoc());
```
- **`std::move(*FileOrErr)`**：
  - `std::move` 是 C++11 引入的一个标准库函数，用于将对象的生命周期转移给另一个对象。
  - 这里将 `FileOrErr` 中的内存缓冲区（`std::unique_ptr<llvm::MemoryBuffer>`）转移给 `llvm::SourceMgr`。
- **`llvm::SMLoc()`**：
  - `llvm::SMLoc` 是 LLVM 提供的一个源代码位置类，表示源代码中的一个位置。
  - 这里使用默认构造的 `llvm::SMLoc`，表示没有特定的位置信息。
- **`AddNewSourceBuffer`**：
  - `AddNewSourceBuffer` 是 `llvm::SourceMgr` 的一个成员函数，用于添加一个新的源代码缓冲区。
  - 它将内存缓冲区和位置信息添加到源代码管理器中。
  - 这样，解析器就可以通过源代码管理器访问文件内容。

```cpp
auto TheLexer = Lexer(SrcMgr, Diags);
auto TheSema = Sema(Diags);
auto TheParser = Parser(TheLexer, TheSema);
TheParser.parse();
```
创建词法分析器。创建语义分析器。创建解析器。执行语义分析。值得一提的是 `Parser` 只管解析, 遇到错误跳过(错误处理的方式), 然后无论解析成是什么都将结果交给 `Sema` , 由 `Sema` 检查、报错。

## DiagnosticsEngine 类
将消息(变量名、字符串)放在 `Diagnostic.def` 文件中, `DiagnosticsEngine` 采取 `#include` + 宏的技巧将消息获取并管理。很巧妙和实用的管理方式。

## Lexer 类
与上一章类似, 不过多出了 `identifier` `number` `string` `comment` 函数来辅助 `next` 函数。逻辑不变。

## Parser 类
从 `ModuleDeclaration *Parser::parse()` 方法开始, 
```cpp
ModuleDeclaration *Parser::parse() {
  ModuleDeclaration *ModDecl = nullptr;
  parseCompilationUnit(ModDecl);
  return ModDecl;
}
```
每个文件解析成一个 `ModuleDeclaration` 来管理。在 `AST.h` 文件中有三个基类(`Decl` `Expr` `Stmt`)衍生出了所有节点类型，每个节点类型实现了 `static bool classof` 方法以配合 `llvm::dyn_cast` 获取节点类型。
```bash
compilationUnit
    : "MODULE" identifier ";" ( import )* block identifier "." ;
import : ( "FROM" identifier )? "IMPORT" identList ";" ;
block
    : ( declaration )* ( "BEGIN" statementSequence )? "END" ;
declaration
    : "CONST" ( constantDeclaration ";" )*
    | "VAR" ( variableDeclaration ";" )*
    | procedureDeclaration ";" ;
constantDeclaration : identifier "=" expression ;
variableDeclaration : identList ":" qualident ;
qualident : identifier ( "." identifier )* ;
identList : identifier ( "," identifier)* ;
procedureDeclaration
    : "PROCEDURE" identifier ( formalParameters )? ";"
        block identifier ;
formalParameters
    : "(" ( formalParameterList )? ")" ( ":" qualident )? ;
formalParameterList
    : formalParameter (";" formalParameter )* ;
formalParameter : ( "VAR" )? identList ":" qualident ;
statementSequence
    : statement ( ";" statement )* ;
statement
    : qualident ( ":=" expression | ( "(" ( expList )? ")" )? )
    | ifStatement | whileStatement | "RETURN" ( expression )? ;
ifStatement
    : "IF" expression "THEN" statementSequence
        ( "ELSE" statementSequence )? "END" ;
whileStatement
    : "WHILE" expression "DO" statementSequence "END" ;
expList
    : expression ( "," expression )* ;
expression
    : simpleExpression ( relation simpleExpression )? ;
relation
    : "=" | "#" | "<" | "<=" | ">" | ">=" ;
simpleExpression
    : ( "+" | "-" )? term ( addOperator term )* ;
addOperator
    : "+" | "-" | "OR" ;
term
    : factor ( mulOperator factor )* ;
mulOperator
    : "*" | "/" | "DIV" | "MOD" | "AND" ;
factor
    : integer_literal | "(" expression ")" | "NOT" factor
    | qualident ( "(" ( expList )? ")" )? ;
```
然后就是 LL(1) + 递归下降, 这个根据已有的文法, 求出 `first集合` `fellow集合`, 也是比较简单, 不熟悉的朋友可以重点看一下, 值得一提的是在开启新作用域的时候要创建一个 `EnterDeclScope` 的实例, 来建立新的作用域。当 `Parser` 按照文法解析出构建 ast 节点需要的一个个变量, 然后交给 `Sema` 检查对应语法、分析作用域、构建出 ast 节点。

## Sema 类
负责构建各个 ast 节点时进行检查, 如:
```cpp
assert(CurrentScope && "CurrentScope not set");

if (TypeDeclaration *Ty = dyn_cast<TypeDeclaration>(D))

if (!RetTypeDecl && RetType)

// ......
```
然后报告相关错误。

比较有意思的是作用域的处理, 所有的作用域构成了一个链表, `Sema::CurrentScope` 属性指向了当前作用域, 可以通过 `Sema::CurrentScope->getParent()` 获取上一个作用域。每个作用域中包含了该作用域的变量, 定义时插入新的变量, 使用时查找当前以及父级作用域是否含有该变量名, 并检查变量类型是否符合要求。代码比较多不在一一解读, 不太清楚的地方可以调试看看。

## 总结
本章构建出了 ast, 并进行了语法分析, 可以说已经是一个比较完整的项目, 仔细阅读、调试结合一些理论知识后, 帮助你更好的理解前端。