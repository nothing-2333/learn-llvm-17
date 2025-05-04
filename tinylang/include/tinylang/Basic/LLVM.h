#pragma once

#include "llvm/Support/Casting.h"

// 这样写可以少写一些 #include
namespace llvm {
class SMLoc;
class SourceMgr;
template <typename T, typename A> class StringMap;
class StringRef;
class raw_ostream;
}

namespace tinylang {
using llvm::cast;
using llvm::cast_or_null;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::isa;

using llvm::raw_ostream;
using llvm::SMLoc;
using llvm::SourceMgr;
using llvm::StringMap;
using llvm::StringRef;
}

