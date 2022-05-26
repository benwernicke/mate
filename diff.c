#include "diff.h"
#include "ast.h"
#include "error.h"

// ben 05.05.2022 | TOOD: this seems a bit chunky
static bool _diff_is_tagged(ast_t* ast, rptr token)
{
    if (ast_token_is_token_null(ast, token) || ast_token_type(ast, token) == AST_TOKEN_NUM) {
        return 0;
    }
    return ast_token_type(ast, token) == AST_TOKEN_VAR || _diff_is_tagged(ast, ast_token_left(ast, token)) || _diff_is_tagged(ast, ast_token_right(ast, token));
}

static rptr differentiate_(ast_t *restrict from, rptr from_token, ast_t *restrict to)
{
    if (!_diff_is_tagged(from, from_token)) {
        return ast_token_num_cons(to, 0);
    }
    switch (ast_token_type(from, from_token)) {
    case AST_TOKEN_NUM:
        err_error("Memory Corruption");
        break;
    case AST_TOKEN_BOP:
        break;
    case AST_TOKEN_UOP:
        break;
    case AST_TOKEN_VAR:
        break;
    }
    err_error("Memory Corruption");
}

ast_t* differentiate(ast_t* from)
{
    ast_t* to = ast_cons();
    return to;
}
