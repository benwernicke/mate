#include "ast.h"

static rptr _ast_token(ast_t* ast, ast_token_type type, rptr left, rptr right);
static rptr _ast_token_bop(ast_t* ast, ast_token_bop_type type, rptr left, rptr right);
static rptr _ast_token_uop(ast_t* ast, ast_token_uop_type type, rptr arg);
static void _lexer_num(ast_t* ast, char** sp);
static void _lexer_var(ast_t* ast, char** sp);
static void _lexer(ast_t* ast, char* s);
static void _ast_connect_up_helper(ast_t* ast, rptr top, rptr curr);
static rptr _ast_more(ast_t* ast);

static rptr _ast_more(ast_t* ast)
{
    if (ast->used_tokens >= ast->allocated_tokens) {
        ast->allocated_tokens = ast->allocated_tokens * 2 + 1;
        ast->buf = realloc(ast->buf, ast->allocated_tokens * sizeof(ast_token_t));
    }
    ast->used_tokens++;

    // ben 09.04.22 | 0 is not invalid due to relative pointers 
    // ben 21.04.22 | if -1 is no longer nullvalue you need to check ast_token_is_up/left/right_null etc
    ast_token_up_set(ast, ast->used_tokens, -1);
    ast_token_left_set(ast, ast->used_tokens, -1);
    ast_token_right_set(ast, ast->used_tokens, -1);

    return ast->used_tokens;
}

bool ast_token_is_up_null(ast_t* ast, rptr token)
{
    return ast_token_up(ast, token) == -1;
}

bool ast_token_is_left_null(ast_t* ast, rptr token)
{
    return ast_token_left(ast, token) == -1;
}

bool ast_token_is_right_null(ast_t* ast, rptr token)
{
    return ast_token_right(ast, token) == -1;
}

void ast_free(ast_t* ast)
{
    if (ast) {
        free(ast->buf);
        free(ast);
    }
}

ast_t* ast_constructor(void)
{
    ast_t* ast = malloc(sizeof(*ast));
    ast->used_tokens = 0;
    ast->allocated_tokens = 16;
    ast->buf = malloc(ast->allocated_tokens * sizeof(*ast->buf));
    ast->root = -1;
    return ast;
}

rptr ast_token_left(ast_t* ast, rptr token)
{
    return ast->buf[token].left;
}

rptr ast_token_right(ast_t* ast, rptr token)
{
    return ast->buf[token].right;
}

rptr ast_token_up(ast_t* ast, rptr token)
{
    return ast->buf[token].up;
}

void ast_token_up_set(ast_t* ast, rptr token, rptr val)
{
    ast->buf[token].up = val;
}

void ast_token_left_set(ast_t* ast, rptr token, rptr val)
{
    ast->buf[token].left = val;
}

void ast_token_right_set(ast_t* ast, rptr token, rptr val)
{
    ast->buf[token].right = val;
}

static rptr _ast_token(ast_t* ast, ast_token_type type, rptr left, rptr right)
{
    rptr token = _ast_more(ast);
    ast->buf[token].type = type;
    ast->buf[token].left = left;
    ast->buf[token].right = right;
    return token;
}

static rptr _ast_token_bop(ast_t* ast, ast_token_bop_type type, rptr left, rptr right)
{
    rptr token = _ast_token(ast, AST_TOKEN_BOP, left, right);
    ast->buf[token].as.bop = type;
    return token;
}

static rptr _ast_token_uop(ast_t* ast, ast_token_uop_type type, rptr arg)
{
    rptr token = _ast_token(ast, AST_TOKEN_BOP, -1, arg);
    ast->buf[token].as.uop = type;
    return token;
}

rptr ast_token_add(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop(ast, AST_TOKEN_BOP_ADD, left, right);
}

rptr ast_token_sub(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop(ast, AST_TOKEN_BOP_SUB, left, right);
}

rptr ast_token_mul(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop(ast, AST_TOKEN_BOP_MUL, left, right);
}

rptr ast_token_div(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop(ast, AST_TOKEN_BOP_DIV, left, right);
}

rptr ast_token_exp(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop(ast, AST_TOKEN_BOP_EXP, left, right);
}

rptr ast_token_neg(ast_t* ast, rptr arg)
{
    return _ast_token_uop(ast, AST_TOKEN_UOP_NEG, arg);
}

rptr ast_token_log(ast_t* ast, rptr arg)
{
    return _ast_token_uop(ast, AST_TOKEN_UOP_LOG, arg);
}

rptr ast_token_sin(ast_t* ast, rptr arg)
{
    return _ast_token_uop(ast, AST_TOKEN_UOP_SIN, arg);
}

rptr ast_token_cos(ast_t* ast, rptr arg)
{
    return _ast_token_uop(ast, AST_TOKEN_UOP_COS, arg);
}

rptr ast_token_tan(ast_t* ast, rptr arg)
{
    return _ast_token_uop(ast, AST_TOKEN_UOP_TAN, arg);
}

rptr ast_token_asin(ast_t* ast, rptr arg)
{
    return _ast_token_uop(ast, AST_TOKEN_UOP_ASIN, arg);
}

rptr ast_token_acos(ast_t* ast, rptr arg)
{
    return _ast_token_uop(ast, AST_TOKEN_UOP_ACOS, arg);
}

rptr ast_token_atan(ast_t* ast, rptr arg)
{
    return _ast_token_uop(ast, AST_TOKEN_UOP_ATAN, arg);
}

rptr ast_token_num(ast_t* ast, double num)
{
    rptr token = _ast_token(ast, AST_TOKEN_NUM, -1, -1);
    ast->buf[token].as.num = num;
    return token;
}

rptr ast_token_var(ast_t* ast, char* s)
{
    rptr token = _ast_token(ast, AST_TOKEN_VAR, -1, -1);
    strcpy(ast->buf[token].as.var, s);
    return token;
}

static void _lexer_num(ast_t* ast, char** sp)
{
    char* s = *sp;
    while (**sp && isdigit(**sp) || **sp == '.') {
        (*sp)++;
    }
    char tmp = **sp;
    **sp = 0;
    ast_token_num(ast, atof(s));
    **sp = tmp;
    (*sp)--;
}

static void _lexer_var(ast_t* ast, char** sp)
{
    char* s = *sp;
    while (**sp && isdigit(**sp) && isalpha(**sp) || **sp == '_') {
        (*sp)++;
    }
    char tmp = **sp;
    **sp = 0;
    ast_token_var(ast, s);
    **sp = tmp;
    (*sp)--;
}

static void _lexer(ast_t* ast, char* s)
{
    while (*s) {
        if (*s == '+') {
            ast_token_add(ast, -1, -1);
        } else if (*s == '-') {
            ast_token_sub(ast, -1, -1);
        } else if (*s == '*') {
            ast_token_mul(ast, -1, -1);
        } else if (*s == '/') {
            ast_token_div(ast, -1, -1);
        } else if (*s == '^') {
            ast_token_exp(ast, -1, -1);
        } else if (*s == '(') {
            _ast_token(ast, AST_TOKEN_PAR_START, -1, -1);
        } else if (*s == ')') {
            _ast_token(ast, AST_TOKEN_PAR_CLOSE, -1, -1);
        } else if (isdigit(*s)) {
            _lexer_num(ast, &s);
        } else if (isalpha(*s)) {
        }
        s++;
    }
}

static void _ast_connect_up_helper(ast_t* ast, rptr top, rptr curr)
{
    ast_token_up_set(ast, curr, top);

    if(!ast_token_is_left_null(ast, curr))
    {
        _ast_connect_up_helper(ast, curr, ast_token_left(ast, curr));
    }

    if(!ast_token_is_right_null(ast, curr))
    {
        _ast_connect_up_helper(ast, curr, ast_token_right(ast, curr));
    }
}

void ast_connect_up(ast_t* ast)
{
    _ast_connect_up_helper(ast, -1, ast->root);
}
