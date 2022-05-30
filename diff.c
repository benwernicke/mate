#include "diff.h"
#define TODO err_error("not implemented!");

// ben 05.05.2022 | TOOD: this seems a bit chunky

inline static bool diff_is_tagged_(ast_t* ast, rptr token, char* var)
{
    if (ast_token_is_token_null(ast, token) || ast_token_type(ast, token) == AST_TOKEN_NUM) {
        return 0;
    }
    if (ast_token_type(ast, token) == AST_TOKEN_VAR) {
        return strcmp(ast_token_ptr(ast, token)->as.var, var) == 0;
    }
    return diff_is_tagged_(ast, ast_token_left(ast, token), var) || diff_is_tagged_(ast, ast_token_right(ast, token), var);
}

static rptr diff_(ast_t* restrict from, const rptr from_token, ast_t* restrict to, char* restrict var);

inline static rptr diff_add_(ast_t* from, rptr from_token, ast_t* to, char* var)
{
    return ast_token_add_cons(to,
        diff_(from, ast_token_left(from, from_token), to, var),
        diff_(from, ast_token_right(from, from_token), to, var));
}

inline static rptr diff_sub_(ast_t* from, rptr from_token, ast_t* to, char* var)
{
    return ast_token_sub_cons(to,
        diff_(from, ast_token_left(from, from_token), to, var),
        diff_(from, ast_token_right(from, from_token), to, var));
}

inline static rptr diff_mul_(ast_t* from, rptr from_token, ast_t* to, char* var)
{
    return ast_token_add_cons(to,
        ast_token_mul_cons(to,
            ast_copy_down(from, ast_token_left(from, from_token), to),
            diff_(from, ast_token_right(from, from_token), to, var)),
        ast_token_mul_cons(to,
            diff_(from, ast_token_left(from, from_token), to, var),
            ast_copy_down(from, ast_token_right(from, from_token), to)));
}

inline static rptr diff_div_(ast_t* from, rptr from_token, ast_t* to, char* var)
{
    return ast_token_div_cons(to,
        ast_token_sub_cons(to,
            ast_token_mul_cons(to,
                diff_(from, ast_token_left(from, from_token), to, var),
                ast_copy_down(from, ast_token_right(from, from_token), to)),
            ast_token_mul_cons(to,
                ast_copy_down(from, ast_token_left(from, from_token), to),
                diff_(from, ast_token_right(from, from_token), to, var))),
        ast_token_exp_cons(to,
            ast_copy_down(from, ast_token_right(from, from_token), to),
            ast_token_num_cons(to, 2)));
}

inline static rptr diff_exp_(ast_t* from, rptr from_token, ast_t* to, char* var)
{
    return ast_token_mul_cons(to,
        ast_copy_down(from, from_token, to),
        ast_token_add_cons(to,
            ast_token_div_cons(to,
                ast_token_mul_cons(to,
                    diff_(from, ast_token_left(from, from_token), to, var),
                    ast_copy_down(from, ast_token_right(from, from_token), to)),
                ast_copy_down(from, ast_token_left(from, from_token), to)),
            ast_token_mul_cons(to,
                ast_token_log_cons(to,
                    ast_copy_down(from, ast_token_left(from, from_token), to)),
                diff_(from, ast_token_right(from, from_token), to, var))));
}

inline static rptr diff_log_(ast_t* from, rptr from_token, ast_t* to, char* var)
{
    return ast_token_div_cons(to,
        diff_(from, ast_token_left(from, from_token), to, var),
        ast_copy_down(from, ast_token_left(from, from_token), to));
}

static rptr diff_(ast_t* restrict from, const rptr from_token, ast_t* restrict to, char* restrict var)
{
    if (ast_token_is_token_null(from, from_token)) {
        err_error("semantic");
    }
    if (!diff_is_tagged_(from, from_token, var)) {
        return ast_token_num_cons(to, 0);
    }
    switch (ast_token_type(from, from_token)) {
    case AST_TOKEN_NUM:
        err_error("Memory Corruption");
        break;
    case AST_TOKEN_BOP:
        switch (ast_token_ptr(from, from_token)->as.bop) {
        case AST_TOKEN_BOP_ADD:
            return diff_add_(from, from_token, to, var);
        case AST_TOKEN_BOP_SUB:
            return diff_sub_(from, from_token, to, var);
        case AST_TOKEN_BOP_MUL:
            return diff_mul_(from, from_token, to, var);
        case AST_TOKEN_BOP_DIV:
            return diff_div_(from, from_token, to, var);
        case AST_TOKEN_BOP_EXP:
            return diff_exp_(from, from_token, to, var);
        case AST_TOKEN_BOP_EQ:
            err_error("you cant differentiate \'=\'");
        }
    case AST_TOKEN_UOP:
        switch (ast_token_ptr(from, from_token)->as.uop) {
        case AST_TOKEN_UOP_NEG:
            TODO;
        case AST_TOKEN_UOP_LOG:
            return diff_log_(from, from_token, to, var);
            TODO;
        case AST_TOKEN_UOP_SIN:
            TODO;
        case AST_TOKEN_UOP_COS:
            TODO;
        case AST_TOKEN_UOP_TAN:
            TODO;
        case AST_TOKEN_UOP_ASIN:
            TODO;
        case AST_TOKEN_UOP_ACOS:
            TODO;
        case AST_TOKEN_UOP_ATAN:
            TODO;
        }
    case AST_TOKEN_VAR:
        return ast_token_num_cons(to, 1); // ben 27.05.22 | don't need to check strcmp(token->var, var) bcs checked earlier
    }
    err_error("Memory Corruption");
}

ast_t* differentiate(ast_t* from, char* var)
{
    ast_t* to = ast_cons(from->used_tokens * 2); // ben 30.05.22 | performance > space
    to->root = diff_(from, from->root, to, var);
    return to;
}
