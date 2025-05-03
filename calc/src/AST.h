#pragma once

#include "llvm/ADT/SmallVector.h"
#include <llvm/ADT/StringRef.h>

class AST;
class Expr;
class Factor;
class BinaryOp;
class WithDecl;

class ASTVisitor
{
public:
    virtual void visit(AST &) {};
    virtual void visit(Expr &) {};
    virtual void visit(Factor &) = 0;
    virtual void visit(BinaryOp &) = 0;
    virtual void visit(WithDecl &) = 0;
};

class AST 
{
public:
    virtual ~AST() {}
    /*
    本质是为了配合多态，当我定义一个 AST 子类时，使用多态，这时用 visitor.visit 只能到 visit(AST &),
    而在 AST 子类实现一个 accept 相当一记录一个自己的类型
    */ 
    virtual void accept(ASTVisitor &visitor) = 0;
};

class Expr : public AST
{
public:
    Expr() {}
};

class Factor : public Expr
{
public:
    enum ValueKind { Ident, Number };

private:
    ValueKind kind;
    llvm::StringRef val;

public:
    Factor(ValueKind kind, llvm::StringRef val) : kind(kind), val(val) {}
    ValueKind get_kind() { return kind; }
    llvm::StringRef get_val() { return val; }
    virtual void accept(ASTVisitor &v) override
    {
        v.visit(*this);
    }
};

class BinaryOp : public Expr
{
public:
    enum Operator { Plus, Minus, Mul, Div };

private:
    Expr *left;
    Expr *right;
    Operator op;

public:
    BinaryOp(Operator op, Expr *left, Expr *right) : op(op), left(left), right(right) {}

    Expr *get_left() { return left; }
    Expr *get_right() { return right; }
    Operator get_operator() { return op; }
    virtual void accept(ASTVisitor &v) override
    {
        v.visit(*this);
    }
};

class WithDecl : public AST
{
    using VarVector = llvm::SmallVector<llvm::StringRef, 8>;
    VarVector vars;
    Expr *expr;

public:
    WithDecl(llvm::SmallVector<llvm::StringRef, 8> vars, Expr *expr) : vars(vars), expr(expr) {}
    VarVector::const_iterator begin() { return vars.begin(); }
    VarVector::const_iterator end() { return vars.end(); }
    Expr* get_expr() { return expr; }
    virtual void accept(ASTVisitor &v) override
    {
        v.visit(*this); 
    }
};