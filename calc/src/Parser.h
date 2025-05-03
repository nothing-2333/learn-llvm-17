#pragma once

#include "Lexer.h"
#include "AST.h"
#include "llvm/Support//raw_ostream.h"

/*
LLVM的编码指南禁止使用 <iostream> 库
*/

class Parser
{
    Lexer &lexer;
    Token token;        // 储存下一个标记（预检）
    bool has_error_;

    void error()
    {
        llvm::errs() << "unexpected: " << token.get_text() << "\n";
        has_error_ = true;
    }

    void advance() { return lexer.next(token); }

    // expect 与 consume 函数的返回值有些反直觉
    bool expect(Token::TokenKind kind)
    {
        if (token.is(kind)) return false;

        error();
        return true;
    }

    bool consume(Token::TokenKind kind)
    {
        if (expect(kind)) return true;

        advance();
        return false;
    }

    AST *parse_calc();
    Expr *parse_expr();
    Expr *parse_term();
    Expr *parse_factor();

public:
    Parser(Lexer &lexer) : lexer(lexer), has_error_(false)
    {
        advance();
    }

    bool has_error() { return has_error_; }

    AST *parse();
};