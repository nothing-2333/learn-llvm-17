#include "Sema.h"
#include "AST.h"
#include "llvm/ADT/StringSet.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

// 语义分析

namespace {
    class DeclCheck : public ASTVisitor
    {
        llvm::StringSet<> scope;
        bool has_error_;
        enum ErrorType { Twice, Not };
        void error(ErrorType error_type, llvm::StringRef v)
        {
            llvm::errs() << "Variable " << v << " " << (error_type == Twice ? "already" : "not") << " declared\n";
            has_error_ = true;
        }

    public:
        DeclCheck() : has_error_(false) {}
        bool has_error() { return has_error_; }

        virtual void visit(Factor &Node) override
        {
            if (Node.get_kind() == Factor::Ident)
            {
                if (scope.find(Node.get_val()) == scope.end()) error(Not, Node.get_val());
            }
        }
        virtual void visit(BinaryOp &Node) override
        {
            if (Node.get_left()) Node.get_left()->accept(*this);
            else has_error_ = true;

            if (Node.get_right()) Node.get_right()->accept(*this);
            else has_error_ = true;
        }
        virtual void visit(WithDecl &Node) override
        {
            for (auto begin = Node.begin(), end = Node.end(); begin != end; ++begin)
            {
                if (!scope.insert(*begin).second) error(Twice, *begin);
            }

            if (Node.get_expr()) Node.get_expr()->accept(*this);
            else has_error_ = true;
        }
    };
};

bool Sema::semantic(AST *tree)
{
    if (!tree) return false;

    DeclCheck check;
    tree->accept(check);
    return check.has_error();
}