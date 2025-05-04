#include "tinylang/Basic/Diagnostic.h"

using namespace tinylang;

namespace 
{
const char *diagnostic_text[] = {
#define DIAG(ID, Level, Msg) Msg,
#include "tinylang/Basic/Diagnostic.def"
};

SourceMgr::DiagKind diagnostic_kind[] = {
#define DIAG(ID, Level, Msg) SourceMgr::DK_##Level,
#include "tinylang/Basic/Diagnostic.def"
};
};

const char *DiagnosticsEngine::get_diagnostic_text(unsigned diag_id)
{
    return diagnostic_text[diag_id];
}

SourceMgr::DiagKind DiagnosticsEngine::get_diagnostic_kind(unsigned diag_id)
{
    return diagnostic_kind[diag_id];
}