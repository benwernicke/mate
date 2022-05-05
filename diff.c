#include "diff.h"
#include "ast.h"

// ben 05.05.2022 | TOOD: this seems a bit chunky
static bool _diff_is_tagged(ast_t* ast, rptr token)
{
    if(token == -1)
    {
        return 0;
    }
    if (ast_token_type(ast, token) == AST_TOKEN_VAR) {
        return 1;
    }
    if (ast_token_type(ast, token) == AST_TOKEN_NUM) {
        return 0;
    }
    return _diff_is_tagged(ast, ast_token_left(ast, token)) || _diff_is_tagged(ast, ast_token_right(ast, token));
}

static rptr diff_helper(ast_t* ast, rptr token)
{
    switch (ast_token_type(ast, token)) {
    case AST_TOKEN_NUM:
        break;
    case AST_TOKEN_BOP:
        break;
    case AST_TOKEN_UOP:
        break;
    case AST_TOKEN_VAR:
        break;
    }
}

ast_t* differentiate(ast_t* ast)
{
}
