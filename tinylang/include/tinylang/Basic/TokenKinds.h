#pragma once

#include "llvm/Support/Compiler.h"

namespace tinylang 
{
namespace tok 
{
enum TokenKind : unsigned short
{
#define TOK(ID) ID,
#include "TokenKinds.def"
    NUM_TOKENS // 定义为枚举的最后一个成员, 表示已定义的标记的数量
};
/*
LLVM_READNONE：函数不会读取任何内存。
LLVM_WRITENONE：函数不会写入任何内存。
LLVM_PURE：函数不会读取或写入任何内存，但可能会有其他副作用（如 I/O 操作）。
LLVM_CONST：函数不会读取或写入任何内存，并且没有其他副作用。
*/
const char *get_token_name(TokenKind kind) LLVM_READNONE;
const char *get_punctuator_spelling(TokenKind kind) LLVM_READNONE;
const char *get_keyword_spelling(TokenKind kind) LLVM_READNONE;
}
}

