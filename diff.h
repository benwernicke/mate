#ifndef DIFF_H
#define DIFF_H
#include "ast.h"

#include "error.h"
#include <string.h>

ast_t* differentiate(ast_t* from, char* var);

#endif
