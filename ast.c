#include "ast.h"

rptr ast_asynth(ast_t* eb, expr_t* token)
{
    return (token - eb->buf);
}

expr_t* ast_synth(ast_t* eb, rptr token)
{
    return eb->buf + token;
}

expr_t* ast_more(ast_t* eb)
{
    if (eb->buf == NULL || eb->used >= eb->alloced) {
        eb->buf = realloc(eb->buf, (eb->alloced <<= 1) * sizeof(*eb->buf));
    }
    expr_t* token = eb->buf + eb->used++;
    token->up = token->l = token->r = -1;
    return token;
}

rptr ast_rmore(ast_t* eb)
{
    return ast_asynth(eb, ast_more(eb));
}

expr_t* token_l(ast_t* eb, expr_t* token)
{
    if (token->l == -1) {
        return NULL;
    }
    return eb->buf + token->l;
}

expr_t* token_r(ast_t* eb, expr_t* token)
{
    if (token->r == -1) {
        return NULL;
    }
    return eb->buf + token->r;
}

expr_t* token_up(ast_t* eb, expr_t* token)
{
    if (token->up == -1) {
        return NULL;
    }
    return eb->buf + token->up;
}
