#pragma once

#include "tinylang/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

namespace tinylang
{
namespace diag 
{
enum
{
#define DIAG(ID, Level, Msg) ID,
#include "tinylang/Basic/Diagnostic.def"
};
}

class DiagnosticsEngine
{
    static const char *get_diagnostic_text(unsigned diag_id);
    static SourceMgr::DiagKind get_diagnostic_kind(unsigned diag_id);

    SourceMgr &source_mgr;
    unsigned num_errors_;

public:
    DiagnosticsEngine(SourceMgr &source_mgr) : source_mgr(source_mgr), num_errors_(0) {}
    unsigned num_errors() { return num_errors_; }

    template<typename... Args>
    void report(SMLoc loc, unsigned diag_id, Args &&... arguments)
    {
        std::string msg = llvm::formatv(get_diagnostic_text(diag_id), std::forward<Args>(arguments)...).str();
        SourceMgr::DiagKind kind = get_diagnostic_kind(diag_id);
        source_mgr.PrintMessage(loc, kind, msg);
        num_errors_ += (kind == SourceMgr::DK_Error);
    }
};
}