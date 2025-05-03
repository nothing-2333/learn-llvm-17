#include "AST.h"
#include "CodeGen.h"
#include "Lexer.h"
#include "Parser.h"
#include "Sema.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"

static llvm::cl::opt<std::string> input(llvm::cl::Positional, llvm::cl::desc("<input expression>"), llvm::cl::init(""));

int main(int argc, const char **argv)
{
    // 首先初始化LLVM库。需要 ParseCommandLineOptions() 来处理命令行上给出的选项
    llvm::InitLLVM x(argc, argv);
    llvm::cl::ParseCommandLineOptions(argc, argv, "calc - the expression compiler\n");

    // 调用词法分析器和语法分析器
    Lexer lexer(input);
    Parser parser(lexer);
    AST *tree = parser.parse();
    if (!tree || parser.has_error())
    {
        llvm::errs() << "Syntax errors occured\n";
        return 1;
    }

    // 有语义错误
    Sema semantic;
    if (semantic.semantic(tree))
    {
        llvm::errs() << "Syntax errors occured\n";
        return 1;
    }

    CodeGen code_generator;
    code_generator.compile(tree);
    return 0;
}