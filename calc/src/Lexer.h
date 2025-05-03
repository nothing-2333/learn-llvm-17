#pragma once

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"

class Lexer;

class Token 
{
    friend class Lexer;

public:
    enum TokenKind : unsigned short
    {
        eoi, unknown, ident, number, comma, colon, plus,
        minus, star, slash, l_paren, r_paren, KW_with
    };

private:
    TokenKind kind;
    llvm::StringRef text;

public:
    TokenKind get_kind() const { return kind; }
    llvm::StringRef get_text() const { return text; }

    bool is(TokenKind k) const { return kind == k; }
    bool is_one_of(TokenKind k1, TokenKind k2) const { return is(k1) || is(k2); }
    template<typename... Ts>
    bool is_one_of(TokenKind k1, TokenKind k2, Ts... ks) const { return is(k1) || is_one_of(k2, ks...); }
};

class Lexer
{
    const char *buffer_start;
    const char *buffer_ptr;

public:
    Lexer(const llvm::StringRef &buffer)
    {
        buffer_start = buffer.begin();
        buffer_ptr = buffer_start;
    }

    void next(Token &token);

private:
    void from_token(Token &token, const char *token_end, Token::TokenKind kind);
};