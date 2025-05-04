#include "tinylang/Basic/TokenKinds.h"
#include "llvm/Support/ErrorHandling.h"

using namespace tinylang;

static const char * const tok_names[] = {
#define TOK(ID) #ID,
#define KEYWORD(ID, FLAG) #ID,
#include "tinylang/Basic/TokenKinds.def"
    nullptr
};

const char *tok::get_token_name(TokenKind kind)
{
    if (kind < tok::NUM_TOKENS) return tok_names[kind];
    llvm_unreachable("unknown TokenKind"); // LLVM 提供的一个宏，用于标记代码中的不可到达区域
    return nullptr;
}

const char *tok::get_punctuator_spelling(TokenKind kind)
{
    switch (kind) 
    {
#define PUNCTUATOR(ID, SP) case ID: return SP;
#include "tinylang/Basic/TokenKinds.def"
        default: break;
    }
    return nullptr;
}

const char *tok::get_keyword_spelling(TokenKind kind)
{
    switch (kind) 
    {
#define KEYWORD(ID, FLAG) case kw_ ## ID: return #ID;
#include "tinylang/Basic/TokenKinds.def"
        default: break;
    }
    return nullptr;
}

