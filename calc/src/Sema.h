#pragma once

#include "AST.h"
#include "Lexer.h"

class Sema
{
public:
    bool semantic(AST *tree);
};