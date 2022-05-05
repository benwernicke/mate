#include "diff.h"
#include "ast.h"

// ben 05.05.2022 | TOOD: this seems a bit chunky
static bool _diff_is_tagged(ast_t* ast, rptr token)
{
    if (ast_token_is_token_null(ast, token) || ast_token_type(ast, token) == AST_TOKEN_NUM) {
        return 0;
    }
    return ast_token_type(ast, token) == AST_TOKEN_VAR || _diff_is_tagged(ast, ast_token_left(ast, token)) || _diff_is_tagged(ast, ast_token_right(ast, token));
}
