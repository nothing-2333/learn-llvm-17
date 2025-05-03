#include "Parser.h"
#include "Lexer.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>

AST *Parser::parse()
{
    AST *res = parse_calc();

    expect(Token::eoi);
    return res;
}

AST *Parser::parse_calc()
{
    // calc : ("with" ident ("," ident)* ":")? expr ;

    Expr *expr;
    llvm::SmallVector<llvm::StringRef, 8> vars;

    if (token.is(Token::KW_with)) 
    {
        advance();
        if (expect(Token::ident)) goto _error;
        vars.push_back(token.get_text());
        advance();
    
        while (token.is(Token::comma))
        {
            advance();
            if (expect(Token::ident)) goto _error;
            vars.push_back(token.get_text());
            advance();
        }

        if (consume(Token::colon)) goto _error;
    }
    expr = parse_expr();

    if (vars.empty()) return expr;
    else return new WithDecl(vars, expr);

_error:
    while (!token.is(Token::eoi)) advance();
    return nullptr;
}

Expr *Parser::parse_expr()
{
    // expr : term (( "+" | "-" ) term)* ;

    Expr *left = parse_term();
    while (token.is_one_of(Token::plus, Token::minus)) {
        BinaryOp::Operator Op = token.is(Token::plus) ? BinaryOp::Plus : BinaryOp::Minus;
        advance();
        Expr *right = parse_term();
        left = new BinaryOp(Op, left, right);
    }
    return left;
}

Expr *Parser::parse_term()
{
    // term : factor (( "*" | "/") factor)* ;
    
    // 乘除法优先级更高，所以单独优先解析
    Expr *left = parse_factor();
    while (token.is_one_of(Token::star, Token::slash)) {
        BinaryOp::Operator op = token.is(Token::star) ? BinaryOp::Mul : BinaryOp::Div;
        advance();
        Expr *right = parse_term();
        left = new BinaryOp(op, left, right);
    }
    return left;
}

Expr *Parser::parse_factor()
{
    // factor : ident | number | "(" expr ")" ;

    Expr *res = nullptr;
    switch (token.get_kind()) {
        case Token::number:
            res = new Factor(Factor::Number, token.get_text());
            advance();
            break;
        case Token::ident:
            res = new Factor(Factor::Ident, token.get_text());
            advance();
            break;
        case Token::l_paren:
            advance();
            res = parse_expr();
            if (!consume(Token::r_paren)) break;
        default:
            // parse_expr() 可能返回 nullptr
            // Token::r_paren, Token::star, Token::plus, Token::minus, Token::slash, Token::eoi 是 factor 的 follow 集合
            if (!res) error();
            while (!token.is_one_of(Token::r_paren, Token::star, Token::plus, Token::minus, Token::slash, Token::eoi)) {
                advance();
            }
    }
    return res;
}
