#pragma once

#include "tinylang/Basic/LLVM.h"
#include "tinylang/Basic/TokenKinds.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/SMLoc.h"

namespace tinylang 
{
class Lexer;

class Token 
{
    friend class Lexer;
    const char *ptr;
    size_t length;
    tok::TokenKind kind;

public:
    tok::TokenKind get_kind() const { return kind; }
    void set_kind(tok::TokenKind k) { kind = k; }
    size_t get_length() const { return length; }

    bool is(tok::TokenKind k) const { return kind == k; }
    bool is_not(tok::TokenKind k) const { return kind != k; }
    template<typename... Tokens>
    bool is_one_of(Tokens &&... toks) const { return (... || is(toks)); } // 语法糖

    const char *get_name() const { return tok::get_token_name(kind); }
    SMLoc get_location() const { return SMLoc::getFromPointer(ptr); }
    StringRef get_identifier()
    {
        assert(is(tok::identifier) && "Cannot get identfier of non-identifier");
        return StringRef(ptr, length);
    }

    StringRef get_literal_data()
    {
        assert(is_one_of(tok::integer_literal, tok::string_literal) && "Cannot get literal data of non-literal");
        return StringRef(ptr, length);
    }
};
}