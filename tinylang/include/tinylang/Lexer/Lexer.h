#pragma once

#include "tinylang/Basic/Diagnostic.h"
#include "tinylang/Basic/LLVM.h"
#include "tinylang/Lexer/Token.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"


namespace tinylang 
{
class KeywordFilter
{
    StringMap<tok::TokenKind> hash_table;
    void add_keyword(StringRef keyword, tok::TokenKind token_code);

public:
    void add_keywords(); // 加载所有关键字
    tok::TokenKind get_keyword(StringRef name, tok::TokenKind default_token_code = tok::unknown)
    {
        auto result = hash_table.find(name);
        if (result != hash_table.end()) return result->second;
        return default_token_code;
    }
};

class Lexer
{
    SourceMgr &src_mgr;
    DiagnosticsEngine &diags;

    const char *cur_ptr;
    StringRef cur_buf;

    unsigned cur_buffer = 0; // 文件标识符

    KeywordFilter key_words;

public:
    Lexer(SourceMgr &src_mgr, DiagnosticsEngine &diags) : src_mgr(src_mgr), diags(diags)
    {
        cur_buffer = src_mgr.getMainFileID();
        cur_buf = src_mgr.getMemoryBuffer(cur_buffer)->getBuffer();
        cur_ptr = cur_buf.begin();
        key_words.add_keywords();
    }

    DiagnosticsEngine &get_diagnostics() const { return diags; }
    StringRef get_buffer() const { return cur_buf; }

    void next(Token &result);    // 返回下一个 token

private:
    void identifier(Token &result);
    void number(Token &result);
    void string(Token &result);
    void comment();

    SMLoc get_loc() { return SMLoc::getFromPointer(cur_ptr); }

    void from_token(Token &result, const char *end, tok::TokenKind kind);
};
}