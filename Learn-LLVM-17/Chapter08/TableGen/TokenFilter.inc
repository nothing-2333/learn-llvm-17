/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Token Kind and Keyword Filter Implementation Fragment                      *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_TOKEN_FLAGS
#undef GET_TOKEN_FLAGS
#endif
#ifdef GET_TOKEN_KIND_DECLARATION
#undef GET_TOKEN_KIND_DECLARATION
namespace tok {
  enum TokenKind : unsigned short {
    NUM_TOKENS
  };
  const char *getTokenName(TokenKind Kind) LLVM_READNONE;
  const char *getPunctuatorSpelling(TokenKind Kind) LLVM_READNONE;
  const char *getKeywordSpelling(TokenKind Kind) LLVM_READNONE;
}
#endif
#ifdef GET_TOKEN_KIND_DEFINITION
#undef GET_TOKEN_KIND_DEFINITION
static const char * const TokNames[] = {
};

const char *tok::getTokenName(TokenKind Kind) {
  if (Kind <= tok::NUM_TOKENS)
    return TokNames[Kind];
  llvm_unreachable("unknown TokenKind");
  return nullptr;
};

const char *tok::getPunctuatorSpelling(TokenKind Kind) {
  switch (Kind) {
    default: break;
  }
  return nullptr;
};

const char *tok::getKeywordSpelling(TokenKind Kind) {
  switch (Kind) {
    default: break;
  }
  return nullptr;
};

#endif
