#include "Lexer.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Compiler.h>

namespace charinfo {
    LLVM_READNONE inline bool is_with_espace(char c)
    {
        return c == ' ' || c == '\t' || c == '\f' ||
        c == '\v' ||
        c == '\r' || c == '\n';
    }

    LLVM_READNONE inline bool is_digit(char c)
    {
        return c >= '0' && c <= '9';
    }

    LLVM_READNONE inline bool is_letter(char c)
    {
        return (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z');
    }
}

void Lexer::next(Token &token)
{
    // 处理空白字符
    while (*buffer_ptr && charinfo::is_with_espace(*buffer_ptr)) {
        ++buffer_ptr;
    }

    // 处理结尾
    if (!*buffer_ptr)
    {
        token.kind = Token::eoi;
        return;
    }

    // 处理标识符
    if (charinfo::is_letter(*buffer_ptr))
    {
        const char *end = buffer_ptr + 1;
        while (charinfo::is_letter(*end)) ++end;
        llvm::StringRef name(buffer_ptr, end - buffer_ptr);
        Token::TokenKind kind = name == "with" ? Token::KW_with : Token::ident;
        from_token(token, end, kind);
        return;
    }
    // 处理数字
    else if (charinfo::is_digit(*buffer_ptr)) {
        const char *end = buffer_ptr + 1;
        while (charinfo::is_digit(*end)) ++end;
        from_token(token, end, Token::number);
        return;
    }
    // 处理剩余字符
    else 
    {
        switch (*buffer_ptr) {
            #define CASE(ch, tok) \
            case ch: from_token(token, buffer_ptr + 1, tok); break
            CASE('+', Token::plus);
            CASE('-', Token::minus);
            CASE('*', Token::star);
            CASE('/', Token::slash);
            CASE('(', Token::l_paren);
            CASE(')', Token::r_paren);
            CASE(':', Token::colon);
            CASE(',', Token::comma);
            #undef CASE
            default: from_token(token, buffer_ptr + 1, Token::unknown);
        }
        return;
    }
}

void Lexer::from_token(Token &token, const char *token_end, Token::TokenKind kind)
{
    token.kind = kind;
    token.text = llvm::StringRef(buffer_ptr, token_end - buffer_ptr);
    buffer_ptr = token_end;
}