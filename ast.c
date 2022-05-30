#include "ast.h"

static rptr ast_more_(ast_t* ast);
static rptr ast_token_cons_(ast_t* ast, ast_token_type_t type, rptr left, rptr right);
static rptr ast_token_bop_cons_(ast_t* ast, ast_token_bop_type_t type, rptr left, rptr right);
static rptr ast_token_uop_cons_(ast_t* ast, ast_token_uop_type_t type, rptr arg);
static void lexer_num_(ast_t* ast, char** sp);
static void lexer_char_(ast_t* ast, char** sp);
static void lexer_(ast_t* ast, char* s);
static void ast_build_(ast_t* ast, size_t* precedences);
static rptr ast_build_helper_(ast_t* ast, size_t* precedences, rptr lower, rptr upper);
inline static void ast_printf_(ast_t* ast, rptr token);

static rptr ast_more_(ast_t* ast)
{
    if (ast->used_tokens >= ast->allocated_tokens) {
        ast->allocated_tokens = ast->allocated_tokens * 2 + 1;
        ast->buf = realloc(ast->buf, ast->allocated_tokens * sizeof(ast_token_t));
    }

    // ben 09.04.22 | 0 is not invalid due to relative pointers
    // ben 21.04.22 | if -1 is no longer nullvalue you need to check ast_token_is_left/right_null etc
    ast_token_left_set(ast, ast->used_tokens, -1);
    ast_token_right_set(ast, ast->used_tokens, -1);

    return ast->used_tokens++;
}

bool ast_token_is_left_null(ast_t* ast, rptr token)
{
    return ast_token_is_token_null(ast, ast_token_left(ast, token));
}

bool ast_token_is_token_null(ast_t* ast, rptr token)
{
    return token == -1;
}

bool ast_token_is_right_null(ast_t* ast, rptr token)
{
    return ast_token_is_token_null(ast, ast_token_right(ast, token));
}

ast_token_t* ast_token_ptr(ast_t* ast, rptr token)
{
    return ast->buf + token;
}

void ast_free(ast_t* ast)
{
    if (ast) {
        free(ast->buf);
        free(ast);
    }
}

// ben 30.05.22 | TODO: what do when malloc fail

ast_t* ast_cons(size_t init_buf_size)
{
    assert(init_buf_size > 1);
    ast_t* ast = malloc(sizeof(*ast));
    assert(ast);
    *ast = (ast_t) {
        .used_tokens = 0,
        .allocated_tokens = init_buf_size,
        .root = -1,
    };
    ast->buf = malloc(ast->allocated_tokens * sizeof(*ast->buf));
    assert(ast->buf);
    return ast;
}

rptr ast_token_left(ast_t* ast, rptr token)
{
    return ast_token_ptr(ast, token)->left;
}

rptr ast_token_right(ast_t* ast, rptr token)
{
    return ast_token_ptr(ast, token)->right;
}

void ast_token_left_set(ast_t* ast, rptr token, rptr val)
{
    ast_token_ptr(ast, token)->left = val;
}

void ast_token_right_set(ast_t* ast, rptr token, rptr val)
{
    ast_token_ptr(ast, token)->right = val;
}

static rptr ast_token_cons_(ast_t* ast, ast_token_type_t type, rptr left, rptr right)
{
    rptr token = ast_more_(ast);
    ast_token_ptr(ast, token)->type = type;
    ast_token_ptr(ast, token)->left = left;
    ast_token_ptr(ast, token)->right = right;
    return token;
}

static rptr ast_token_bop_cons_(ast_t* ast, ast_token_bop_type_t type, rptr left, rptr right)
{
    rptr token = ast_token_cons_(ast, AST_TOKEN_BOP, left, right);
    ast->buf[token].as.bop = type;
    return token;
}

static rptr ast_token_uop_cons_(ast_t* ast, ast_token_uop_type_t type, rptr arg)
{
    rptr token = ast_token_cons_(ast, AST_TOKEN_UOP, -1, arg);
    ast->buf[token].as.uop = type;
    return token;
}

rptr ast_token_add_cons(ast_t* ast, rptr left, rptr right)
{
    return ast_token_bop_cons_(ast, AST_TOKEN_BOP_ADD, left, right);
}

rptr ast_token_sub_cons(ast_t* ast, rptr left, rptr right)
{
    return ast_token_bop_cons_(ast, AST_TOKEN_BOP_SUB, left, right);
}

rptr ast_token_mul_cons(ast_t* ast, rptr left, rptr right)
{
    return ast_token_bop_cons_(ast, AST_TOKEN_BOP_MUL, left, right);
}

rptr ast_token_div_cons(ast_t* ast, rptr left, rptr right)
{
    return ast_token_bop_cons_(ast, AST_TOKEN_BOP_DIV, left, right);
}

rptr ast_token_exp_cons(ast_t* ast, rptr left, rptr right)
{
    return ast_token_bop_cons_(ast, AST_TOKEN_BOP_EXP, left, right);
}

rptr ast_token_neg_cons(ast_t* ast, rptr arg)
{
    return ast_token_uop_cons_(ast, AST_TOKEN_UOP_NEG, arg);
}

rptr ast_token_log_cons(ast_t* ast, rptr arg)
{
    return ast_token_uop_cons_(ast, AST_TOKEN_UOP_LOG, arg);
}

rptr ast_token_sin_cons(ast_t* ast, rptr arg)
{
    return ast_token_uop_cons_(ast, AST_TOKEN_UOP_SIN, arg);
}

rptr ast_token_cos_cons(ast_t* ast, rptr arg)
{
    return ast_token_uop_cons_(ast, AST_TOKEN_UOP_COS, arg);
}

rptr ast_token_tan_cons(ast_t* ast, rptr arg)
{
    return ast_token_uop_cons_(ast, AST_TOKEN_UOP_TAN, arg);
}

rptr ast_token_asin_cons(ast_t* ast, rptr arg)
{
    return ast_token_uop_cons_(ast, AST_TOKEN_UOP_ASIN, arg);
}

rptr ast_token_acos_cons(ast_t* ast, rptr arg)
{
    return ast_token_uop_cons_(ast, AST_TOKEN_UOP_ACOS, arg);
}

rptr ast_token_atan_cons(ast_t* ast, rptr arg)
{
    return ast_token_uop_cons_(ast, AST_TOKEN_UOP_ATAN, arg);
}

rptr ast_token_num_cons(ast_t* ast, double num)
{
    rptr token = ast_token_cons_(ast, AST_TOKEN_NUM, -1, -1);
    ast_token_ptr(ast, token)->as.num = num;
    return token;
}

rptr ast_token_var_cons(ast_t* ast, char* s)
{
    rptr token = ast_token_cons_(ast, AST_TOKEN_VAR, -1, -1);
    strcpy(ast_token_ptr(ast, token)->as.var, s);
    return token;
}

static void lexer_num_(ast_t* ast, char** sp)
{
    char* s = *sp;
    while (**sp && (isdigit(**sp) || **sp == '.')) {
        (*sp)++;
    }
    char tmp = **sp;
    **sp = '\0';
    ast_token_num_cons(ast, atof(s));
    **sp = tmp;
    (*sp)--;
}

static void lexer_char_(ast_t* ast, char** sp)
{
    char* s = *sp;
    while (**sp && (isdigit(**sp) || isalpha(**sp) || **sp == '_')) {
        (*sp)++;
    }
    char tmp = **sp;
    **sp = 0;
    char len = strlen(s);
    if (len == 3) {
        if (strcmp(s, "sin") == 0) {
            ast_token_sin_cons(ast, -1);
        } else if (strcmp(s, "cos") == 0) {
            ast_token_cos_cons(ast, -1);
        } else if (strcmp(s, "tan") == 0) {
            ast_token_tan_cons(ast, -1);
        } else if (strcmp(s, "log") == 0) {
            ast_token_log_cons(ast, -1);
        } else {
            ast_token_var_cons(ast, s);
        }
    } else if (len == 4) {
        if (strcmp(s, "asin") == 0) {
            ast_token_asin_cons(ast, -1);
        } else if (strcmp(s, "acos") == 0) {
            ast_token_acos_cons(ast, -1);
        } else if (strcmp(s, "atan") == 0) {
            ast_token_atan_cons(ast, -1);
        } else {
            ast_token_var_cons(ast, s);
        }
    } else {
        ast_token_var_cons(ast, s);
    }
    **sp = tmp;
    (*sp)--;
}

// ben 22.04.22 | TODO: actual difference between '-' as bop and '-' as uop
// ben 27.05.22 | TODO: input validation!!!

static void lexer_(ast_t* ast, char* s)
{
    while (*s) {
        if (*s == '+') {
            ast_token_add_cons(ast, -1, -1);
        } else if (*s == '-') {
            ast_token_sub_cons(ast, -1, -1);
        } else if (*s == '*') {
            ast_token_mul_cons(ast, -1, -1);
        } else if (*s == '/') {
            ast_token_div_cons(ast, -1, -1);
        } else if (*s == '^') {
            ast_token_exp_cons(ast, -1, -1);
        } else if (isdigit(*s)) {
            lexer_num_(ast, &s);
        } else if (isalpha(*s)) {
            lexer_char_(ast, &s);
        }
        s++;
    }
}

static size_t ast_token_raw_precedence_(ast_t* ast, rptr token)
{
    switch (ast_token_type(ast, token)) {
    case AST_TOKEN_BOP:
        switch (ast_token_ptr(ast, token)->as.bop) {
        case AST_TOKEN_BOP_ADD:
        case AST_TOKEN_BOP_SUB:
            return 1;
        case AST_TOKEN_BOP_MUL:
        case AST_TOKEN_BOP_DIV:
            return 2;
        case AST_TOKEN_BOP_EXP:
            return 3;
        case AST_TOKEN_BOP_EQ:
            return 0;
        }
    case AST_TOKEN_UOP:
        return 4;
    case AST_TOKEN_NUM:
    case AST_TOKEN_VAR:
        return 5;
    }
    err_error("Memory Corruption");
}

static void ast_calc_precedences_(ast_t* ast, char* s, size_t* precedence)
{
    size_t par_level = 0;
    rptr token = 0;
    while (*s) {
        if (*s == '(') {
            par_level++;
        } else if (*s == ')') {
            par_level--;
        } else if (*s != ' ') {
            if (ast_token_type(ast, token) == AST_TOKEN_NUM || ast_token_type(ast, token) == AST_TOKEN_VAR) {
                while (*s && (isalpha(*s) || isdigit(*s) || *s == '_' || *s == '.')) {
                    s++;
                }
                s--;
            } else if (ast_token_type(ast, token) == AST_TOKEN_UOP) {
                while (*s && isalpha(*s)) {
                    s++;
                }
                s--;
            }
            precedence[token] = 6 * par_level + ast_token_raw_precedence_(ast, token);
            token++;
        }
        s++;
    }
}

ast_t* ast_ast_from_str(char* s)
{
    ast_t* ast = ast_cons(strlen(s)); // a bit more allocated tokens is better then more mallocs space < performance
    lexer_(ast, s);
    size_t precedences[ast->used_tokens];
    // ben 22.04.22 | we calc precedences of nodes here seperatly to
    //                avoid having to store them in the struct and to
    //                avoid using malloc in the lexer
    //
    //                probably on more iteration over string is
    //                probably faster than a lot of heap allocations
    ast_calc_precedences_(ast, s, precedences);
    ast_build_(ast, precedences);
    return ast;
}

ast_token_type_t ast_token_type(ast_t* ast, rptr token)
{
    return ast_token_ptr(ast, token)->type;
}

void ast_token_print(ast_t* ast, rptr token)
{
    switch (ast_token_type(ast, token)) {
    case AST_TOKEN_NUM:
        printf("%lf ", ast_token_ptr(ast, token)->as.num);
        break;
    case AST_TOKEN_BOP:
        switch (ast_token_ptr(ast, token)->as.bop) {
        case AST_TOKEN_BOP_ADD:
            printf("+ ");
            break;
        case AST_TOKEN_BOP_SUB:
            printf("- ");
            break;
        case AST_TOKEN_BOP_MUL:
            printf("* ");
            break;
        case AST_TOKEN_BOP_DIV:
            printf("/ ");
            break;
        case AST_TOKEN_BOP_EXP:
            printf("^ ");
            break;
        case AST_TOKEN_BOP_EQ:
            printf("= ");
            break;
        }
        break;
    case AST_TOKEN_UOP:
        switch (ast_token_ptr(ast, token)->as.uop) {
        case AST_TOKEN_UOP_NEG:
            printf("- ");
            break;
        case AST_TOKEN_UOP_LOG:
            printf("log ");
            break;
        case AST_TOKEN_UOP_SIN:
            printf("sin ");
            break;
        case AST_TOKEN_UOP_COS:
            printf("cos ");
            break;
        case AST_TOKEN_UOP_TAN:
            printf("tan ");
            break;
        case AST_TOKEN_UOP_ASIN:
            printf("asin ");
            break;
        case AST_TOKEN_UOP_ACOS:
            printf("acos ");
            break;
        case AST_TOKEN_UOP_ATAN:
            printf("atan ");
            break;
        }
        break;
    case AST_TOKEN_VAR:
        printf("%s ", ast_token_ptr(ast, token)->as.var);
        break;
    }
}

void ast_print_buf(ast_t* ast)
{
    rptr token;
    for (token = 0; token < ast->used_tokens; token++) {
        ast_token_print(ast, token);
    }
    printf("\n");
}

static rptr ast_build_helper_(ast_t* ast, size_t* precedences, rptr lower, rptr upper)
{
    if (lower == upper) {
        return -1;
    }
    rptr curr;
    rptr lowest_prec = lower;
    for (curr = lower; curr < upper; curr++) {
        if (precedences[curr] <= precedences[lowest_prec]) {
            lowest_prec = curr;
        }
    }
    ast_token_left_set(ast, lowest_prec, ast_build_helper_(ast, precedences, lower, lowest_prec));
    ast_token_right_set(ast, lowest_prec, ast_build_helper_(ast, precedences, lowest_prec + 1, upper));
    return lowest_prec;
}

static void ast_build_(ast_t* ast, size_t* precedences)
{
    ast->root = ast_build_helper_(ast, precedences, 0, ast->used_tokens);
}

static void ast_dump_ast_helper_(ast_t* ast, rptr token, int level)
{
    if (token == -1) {
        return;
    }
    int i;
    for (i = 0; i < level; i++) {
        printf("\t");
    }
    printf("--->");
    ast_token_print(ast, token);
    printf("\n");

    ast_dump_ast_helper_(ast, ast_token_left(ast, token), level + 1);
    ast_dump_ast_helper_(ast, ast_token_right(ast, token), level + 1);
}

inline static void ast_printf_child_(ast_t* ast, rptr token, rptr child_token)
{
    if (ast_token_is_token_null(ast, child_token)) {
        return;
    }
    if (ast_token_raw_precedence_(ast, child_token) < ast_token_raw_precedence_(ast, token)) {
        printf("( ");
        ast_printf_(ast, child_token);
        printf(") ");
    } else {
        ast_printf_(ast, child_token);
    }
}

inline static void ast_printf_(ast_t* ast, rptr token)
{
    ast_printf_child_(ast, token, ast_token_left(ast, token));
    ast_token_print(ast, token);
    ast_printf_child_(ast, token, ast_token_right(ast, token));
}

// ben 28.05.22 | returns root of copy
rptr ast_copy_down(ast_t* from, rptr from_token, ast_t* to)
{
    if (ast_token_is_token_null(from, from_token)) {
        return -1;
    }
    rptr to_token = ast_more_(to);
    *ast_token_ptr(to, to_token) = *ast_token_ptr(from, from_token);
    ast_token_ptr(to, to_token)->left = ast_copy_down(from, ast_token_left(from, from_token), to);
    ast_token_ptr(to, to_token)->right = ast_copy_down(from, ast_token_right(from, from_token), to);
    return to_token;
}

void ast_printf(ast_t* ast)
{
    ast_printf_(ast, ast->root);
}

void ast_dump_ast(ast_t* ast)
{
    ast_dump_ast_helper_(ast, ast->root, 0);
}
