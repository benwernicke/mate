#include "ast.h"

static rptr ast_more_(ast_t* ast);
static ast_token_t* ast_token_ptr_(ast_t* ast, rptr token);
static rptr ast_token_cons_(ast_t* ast, ast_token_type_t type, rptr left, rptr right);
static rptr ast_token_bop_cons_(ast_t* ast, ast_token_bop_type_t type, rptr left, rptr right);
static rptr ast_token_uop_cons_(ast_t* ast, ast_token_uop_type_t type, rptr arg);
static void lexer_num_(ast_t* ast, char** sp);
static void lexer_char_(ast_t* ast, char** sp);
static void lexer_(ast_t* ast, char* s);
static void ast_build_(ast_t* ast, size_t* precedences);
static rptr ast_build_helper_(ast_t* ast, size_t* precedences, rptr lower, rptr upper);

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

static ast_token_t* ast_token_ptr_(ast_t* ast, rptr token)
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

ast_t* ast_cons(void)
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
    return ast_token_ptr_(ast, token)->left;
}

rptr ast_token_right(ast_t* ast, rptr token)
{
    return ast_token_ptr_(ast, token)->right;
}

void ast_token_left_set(ast_t* ast, rptr token, rptr val)
{
    ast_token_ptr_(ast, token)->left = val;
}

void ast_token_right_set(ast_t* ast, rptr token, rptr val)
{
    ast_token_ptr_(ast, token)->right = val;
}

static rptr ast_token_cons_(ast_t* ast, ast_token_type_t type, rptr left, rptr right)
{
    rptr token = ast_more_(ast);
    ast_token_ptr_(ast, token)->type = type;
    ast_token_ptr_(ast, token)->left = left;
    ast_token_ptr_(ast, token)->right = right;
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
    ast_token_ptr_(ast, token)->as.num = num;
    return token;
}

rptr ast_token_var_cons(ast_t* ast, char* s)
{
    rptr token = ast_token_cons_(ast, AST_TOKEN_VAR, -1, -1);
    strcpy(ast_token_ptr_(ast, token)->as.var, s);
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
            switch (ast_token_type(ast, token)) {
            case AST_TOKEN_BOP:
                switch (ast_token_ptr_(ast, token)->as.bop) {
                case AST_TOKEN_BOP_ADD:
                case AST_TOKEN_BOP_SUB:
                    precedence[token] = 6 * par_level + 1;
                    break;
                case AST_TOKEN_BOP_MUL:
                case AST_TOKEN_BOP_DIV:
                    precedence[token] = 6 * par_level + 2;
                    break;
                case AST_TOKEN_BOP_EXP:
                    precedence[token] = 6 * par_level + 3;
                    break;
                case AST_TOKEN_BOP_EQ:
                    precedence[token] = 6 * par_level + 0;
                    break;
                }
                break;
            case AST_TOKEN_UOP:
                precedence[token] = 6 * par_level + 4;
                while (*s && isalpha(*s)) {
                    s++;
                }
                s--;
                break;
            case AST_TOKEN_NUM:
            case AST_TOKEN_VAR:
                while (*s && (isalpha(*s) || isdigit(*s) || *s == '_' || *s == '.')) {
                    s++;
                }
                s--;
                precedence[token] = 6 * par_level + 5;
                break;
            }
            token++;
        }
        s++;
    }
}

ast_t* ast_ast_from_str(char* s)
{
    ast_t* ast = ast_cons();
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
    return ast_token_ptr_(ast, token)->type;
}

void ast_token_print(ast_t* ast, rptr token)
{
    switch (ast_token_type(ast, token)) {
    case AST_TOKEN_NUM:
        printf("%lf ", ast_token_ptr_(ast, token)->as.num);
        break;
    case AST_TOKEN_BOP:
        switch (ast_token_ptr_(ast, token)->as.bop) {
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
        switch (ast_token_ptr_(ast, token)->as.uop) {
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
        printf("%s ", ast_token_ptr_(ast, token)->as.var);
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

void ast_dump_ast(ast_t* ast)
{
    ast_dump_ast_helper_(ast, ast->root, 0);
}
