#include "ast.h"

static rptr _ast_more(ast_t* ast);
static ast_token_t* _ast_token_ptr(ast_t* ast, rptr token);
static rptr _ast_token_cons(ast_t* ast, ast_token_type_t type, rptr left, rptr right);
static rptr _ast_token_bop_cons(ast_t* ast, ast_token_bop_type_t type, rptr left, rptr right);
static rptr _ast_token_uop_cons(ast_t* ast, ast_token_uop_type_t type, rptr arg);
static void _lexer_num(ast_t* ast, char** sp, size_t par_level);
static void _lexer_char(ast_t* ast, char** sp, size_t par_level);
static void _lexer(ast_t* ast, char* s);
static void _ast_connect_up_helper(ast_t* ast, rptr top, rptr curr);
static void _ast_build(ast_t* ast);
static rptr _ast_build_helper(ast_t* ast, rptr curr, rptr last);
static void _ast_lexer_precedence_set(ast_t* ast);

static rptr _ast_more(ast_t* ast)
{
    if (ast->used_tokens >= ast->allocated_tokens) {
        ast->allocated_tokens = ast->allocated_tokens * 2 + 1;
        ast->buf = realloc(ast->buf, ast->allocated_tokens * sizeof(ast_token_t));
    }

    // ben 09.04.22 | 0 is not invalid due to relative pointers
    // ben 21.04.22 | if -1 is no longer nullvalue you need to check ast_token_is_up/left/right_null etc
    ast_token_up_set(ast, ast->used_tokens, -1);
    ast_token_left_set(ast, ast->used_tokens, -1);
    ast_token_right_set(ast, ast->used_tokens, -1);

    return ast->used_tokens++;
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

static ast_token_t* _ast_token_ptr(ast_t* ast, rptr token)
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
    return _ast_token_ptr(ast, token)->left;
}

rptr ast_token_right(ast_t* ast, rptr token)
{
    return _ast_token_ptr(ast, token)->right;
}

rptr ast_token_up(ast_t* ast, rptr token)
{
    return _ast_token_ptr(ast, token)->up;
}

void ast_token_up_set(ast_t* ast, rptr token, rptr val)
{
    _ast_token_ptr(ast, token)->up = val;
}

void ast_token_left_set(ast_t* ast, rptr token, rptr val)
{
    _ast_token_ptr(ast, token)->left = val;
}

void ast_token_right_set(ast_t* ast, rptr token, rptr val)
{
    _ast_token_ptr(ast, token)->right = val;
}

static rptr _ast_token_cons(ast_t* ast, ast_token_type_t type, rptr left, rptr right)
{
    rptr token = _ast_more(ast);
    _ast_token_ptr(ast, token)->type = type;
    _ast_token_ptr(ast, token)->left = left;
    _ast_token_ptr(ast, token)->right = right;
    return token;
}

static rptr _ast_token_bop_cons(ast_t* ast, ast_token_bop_type_t type, rptr left, rptr right)
{
    rptr token = _ast_token_cons(ast, AST_TOKEN_BOP, left, right);
    ast->buf[token].as.bop = type;
    return token;
}

static rptr _ast_token_uop_cons(ast_t* ast, ast_token_uop_type_t type, rptr arg)
{
    rptr token = _ast_token_cons(ast, AST_TOKEN_UOP, -1, arg);
    ast->buf[token].as.uop = type;
    return token;
}

rptr ast_token_add_cons(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop_cons(ast, AST_TOKEN_BOP_ADD, left, right);
}

rptr ast_token_sub_cons(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop_cons(ast, AST_TOKEN_BOP_SUB, left, right);
}

rptr ast_token_mul_cons(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop_cons(ast, AST_TOKEN_BOP_MUL, left, right);
}

rptr ast_token_div_cons(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop_cons(ast, AST_TOKEN_BOP_DIV, left, right);
}

rptr ast_token_exp_cons(ast_t* ast, rptr left, rptr right)
{
    return _ast_token_bop_cons(ast, AST_TOKEN_BOP_EXP, left, right);
}

rptr ast_token_neg_cons(ast_t* ast, rptr arg)
{
    return _ast_token_uop_cons(ast, AST_TOKEN_UOP_NEG, arg);
}

rptr ast_token_log_cons(ast_t* ast, rptr arg)
{
    return _ast_token_uop_cons(ast, AST_TOKEN_UOP_LOG, arg);
}

rptr ast_token_sin_cons(ast_t* ast, rptr arg)
{
    return _ast_token_uop_cons(ast, AST_TOKEN_UOP_SIN, arg);
}

rptr ast_token_cos_cons(ast_t* ast, rptr arg)
{
    return _ast_token_uop_cons(ast, AST_TOKEN_UOP_COS, arg);
}

rptr ast_token_tan_cons(ast_t* ast, rptr arg)
{
    return _ast_token_uop_cons(ast, AST_TOKEN_UOP_TAN, arg);
}

rptr ast_token_asin_cons(ast_t* ast, rptr arg)
{
    return _ast_token_uop_cons(ast, AST_TOKEN_UOP_ASIN, arg);
}

rptr ast_token_acos_cons(ast_t* ast, rptr arg)
{
    return _ast_token_uop_cons(ast, AST_TOKEN_UOP_ACOS, arg);
}

rptr ast_token_atan_cons(ast_t* ast, rptr arg)
{
    return _ast_token_uop_cons(ast, AST_TOKEN_UOP_ATAN, arg);
}

rptr ast_token_num_cons(ast_t* ast, double num)
{
    rptr token = _ast_token_cons(ast, AST_TOKEN_NUM, -1, -1);
    _ast_token_ptr(ast, token)->as.num = num;
    return token;
}

rptr ast_token_var_cons(ast_t* ast, char* s)
{
    rptr token = _ast_token_cons(ast, AST_TOKEN_VAR, -1, -1);
    strcpy(_ast_token_ptr(ast, token)->as.var, s);
    return token;
}

static void _lexer_num(ast_t* ast, char** sp, size_t par_level)
{
    char* s = *sp;
    while (**sp && (isdigit(**sp) || **sp == '.')) {
        (*sp)++;
    }
    char tmp = **sp;
    **sp = '\0';
    rptr token = ast_token_num_cons(ast, atof(s));
    _ast_token_ptr(ast, token)->precedence = 6 * par_level + 5;
    **sp = tmp;
    (*sp)--;
}

static void _lexer_char(ast_t* ast, char** sp, size_t par_level)
{
    rptr token;
    char* s = *sp;
    while (**sp && (isdigit(**sp) || isalpha(**sp) || **sp == '_')) {
        (*sp)++;
    }
    char tmp = **sp;
    **sp = 0;
    char len = strlen(s);
    if (len == 3) {
        if (strcmp(s, "sin") == 0) {
            token = ast_token_sin_cons(ast, -1);
        } else if (strcmp(s, "cos") == 0) {
            token = ast_token_cos_cons(ast, -1);
        } else if (strcmp(s, "tan") == 0) {
            token = ast_token_tan_cons(ast, -1);
        } else if (strcmp(s, "log") == 0) {
            token = ast_token_log_cons(ast, -1);
        } else {
            token = ast_token_var_cons(ast, s);
        }
    } else if (len == 4) {
        if (strcmp(s, "asin") == 0) {
            token = ast_token_asin_cons(ast, -1);
        } else if (strcmp(s, "acos") == 0) {
            token = ast_token_acos_cons(ast, -1);
        } else if (strcmp(s, "atan") == 0) {
            token = ast_token_atan_cons(ast, -1);
        } else {
            token = ast_token_var_cons(ast, s);
        }
    } else {
        token = ast_token_var_cons(ast, s);
    }
    _ast_token_ptr(ast, token)->precedence = (ast_token_type(ast, token) == AST_TOKEN_VAR) ? 5 : 4;
    _ast_token_ptr(ast, token)->precedence += 6 * par_level;
    **sp = tmp;
    (*sp)--;
}

static void _ast_lexer_add_cons(ast_t* ast, size_t par_level)
{
    rptr token = _ast_token_bop_cons(ast, AST_TOKEN_BOP_ADD, -1, -1);
    _ast_token_ptr(ast, token)->precedence = 6 * par_level + 1;
}

static void _ast_lexer_sub_cons(ast_t* ast, size_t par_level)
{
    rptr token = _ast_token_bop_cons(ast, AST_TOKEN_BOP_SUB, -1, -1);
    _ast_token_ptr(ast, token)->precedence = 6 * par_level + 1;
}

static void _ast_lexer_mul_cons(ast_t* ast, size_t par_level)
{
    rptr token = _ast_token_bop_cons(ast, AST_TOKEN_BOP_MUL, -1, -1);
    _ast_token_ptr(ast, token)->precedence = 6 * par_level + 2;
}

static void _ast_lexer_div_cons(ast_t* ast, size_t par_level)
{
    rptr token = _ast_token_bop_cons(ast, AST_TOKEN_BOP_DIV, -1, -1);
    _ast_token_ptr(ast, token)->precedence = 6 * par_level + 2;
}

static void _ast_lexer_exp_cons(ast_t* ast, size_t par_level)
{
    rptr token = _ast_token_bop_cons(ast, AST_TOKEN_BOP_EXP, -1, -1);
    _ast_token_ptr(ast, token)->precedence = 6 * par_level + 3;
}

// ben 21.04.22 | TODO: do prec calc here
static void _lexer(ast_t* ast, char* s)
{
    size_t par_level = 0;
    while (*s) {
        if (*s == '+') {
            _ast_lexer_add_cons(ast, par_level);
        } else if (*s == '-') {
            _ast_lexer_sub_cons(ast, par_level);
        } else if (*s == '*') {
            _ast_lexer_mul_cons(ast, par_level);
        } else if (*s == '/') {
            _ast_lexer_div_cons(ast, par_level);
        } else if (*s == '^') {
            _ast_lexer_exp_cons(ast, par_level);
        } else if (*s == '(') {
            par_level++;
        } else if (*s == ')') {
            par_level--;
        } else if (isdigit(*s)) {
            _lexer_num(ast, &s, par_level);
        } else if (isalpha(*s)) {
            _lexer_char(ast, &s, par_level);
        }
        s++;
    }
}

ast_t* ast_ast_from_str(char* s)
{
    ast_t* ast = ast_cons();
    _lexer(ast, s);
    _ast_build(ast);
    ast_connect_up(ast);
    return ast;
}

ast_token_type_t ast_token_type(ast_t* ast, rptr token)
{
    return _ast_token_ptr(ast, token)->type;
}

void ast_token_print(ast_t* ast, rptr token)
{
    switch (ast_token_type(ast, token)) {
    case AST_TOKEN_NUM:
        printf("%lf ", _ast_token_ptr(ast, token)->as.num);
        break;
    case AST_TOKEN_BOP:
        switch (_ast_token_ptr(ast, token)->as.bop) {
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
        switch (_ast_token_ptr(ast, token)->as.uop) {
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
        printf("%s ", _ast_token_ptr(ast, token)->as.var);
        break;
    case AST_TOKEN_PAR_START:
        printf("( ");
        break;
    case AST_TOKEN_PAR_CLOSE:
        printf(") ");
        break;
    }
}

static void _ast_connect_up_helper(ast_t* ast, rptr top, rptr curr)
{
    ast_token_up_set(ast, curr, top);

    if (!ast_token_is_left_null(ast, curr)) {
        _ast_connect_up_helper(ast, curr, ast_token_left(ast, curr));
    }

    if (!ast_token_is_right_null(ast, curr)) {
        _ast_connect_up_helper(ast, curr, ast_token_right(ast, curr));
    }
}

void ast_connect_up(ast_t* ast)
{
    _ast_connect_up_helper(ast, -1, ast->root);
}

void ast_print_buf(ast_t* ast)
{
    rptr token;
    for (token = 0; token < ast->used_tokens; token++) {
        ast_token_print(ast, token);
    }
    printf("\n");
}

static bool _ast_lexing_is_num_good(ast_t* ast, rptr token)
{
    return token + 1 == ast->used_tokens || ast_token_type(ast, token + 1) == AST_TOKEN_BOP || ast_token_type(ast, token + 1) == AST_TOKEN_BOP || ast_token_type(ast, token + 1) == AST_TOKEN_PAR_CLOSE;
}

static bool _ast_lexing_is_op_good(ast_t* ast, rptr token)
{
    return (token + 1 < ast->used_tokens) && (ast_token_type(ast, token + 1) == AST_TOKEN_UOP || ast_token_type(ast, token + 1) == AST_TOKEN_NUM || ast_token_type(ast, token + 1) == AST_TOKEN_PAR_START || ast_token_type(ast, token + 1) == AST_TOKEN_VAR);
}

static bool _ast_lexing_is_par_start_good(ast_t* ast, rptr token)
{
    return (token + 1 < ast->used_tokens) && (ast_token_type(ast, token + 1) == AST_TOKEN_UOP || ast_token_type(ast, token + 1) == AST_TOKEN_NUM || ast_token_type(ast, token + 1) == AST_TOKEN_PAR_START || ast_token_type(ast, token + 1) == AST_TOKEN_VAR);
}

static bool _ast_lexing_is_par_close_good(ast_t* ast, rptr token)
{
    return token + 1 == ast->used_tokens || ast_token_type(ast, token + 1) == AST_TOKEN_BOP || ast_token_type(ast, token) == AST_TOKEN_UOP;
}

static bool _ast_lexing_is_token_good(ast_t* ast, rptr token)
{
    switch (ast_token_type(ast, token)) {
    case AST_TOKEN_NUM:
        return _ast_lexing_is_num_good(ast, token);
    case AST_TOKEN_UOP:
    case AST_TOKEN_BOP:
        return _ast_lexing_is_op_good(ast, token);
    case AST_TOKEN_VAR:
        return _ast_lexing_is_num_good(ast, token);
    case AST_TOKEN_PAR_START:
        return _ast_lexing_is_par_start_good(ast, token);
    case AST_TOKEN_PAR_CLOSE:
        return _ast_lexing_is_par_close_good(ast, token);
    }
    return 0;
}

bool ast_lexing_is_semantic_good(ast_t* ast) // ben 21.04.22 | assumes that lexing is not build to tree yet
{
    rptr token = 0;

    for (token = 0; token < ast->used_tokens; token++) {
        if (!_ast_lexing_is_token_good(ast, token)) {
            return 0;
        }
    }
    return 1;
}

static size_t _ast_token_precedence(ast_t* ast, rptr token)
{
    return _ast_token_ptr(ast, token)->precedence;
}

static rptr _ast_build_helper(ast_t* ast, rptr lower, rptr upper)
{
    if (lower == upper) {
        return -1;
    }
    rptr curr;
    rptr lowest_prec = lower;
    for (curr = lower; curr < upper; curr++) {
        if (_ast_token_precedence(ast, curr) <= _ast_token_precedence(ast, lowest_prec)) {
            lowest_prec = curr;
        }
    }
    ast_token_left_set(ast, lowest_prec, _ast_build_helper(ast, lower, lowest_prec));
    ast_token_right_set(ast, lowest_prec, _ast_build_helper(ast, lowest_prec + 1, upper));
    return lowest_prec;
}

static void _ast_build(ast_t* ast)
{
    ast->root = _ast_build_helper(ast, 0, ast->used_tokens);
}

static void _ast_dump_ast_helper(ast_t* ast, rptr token, int level)
{
    if (token == -1) {
        return;
    }
    int i;
    for (i = 0; i < level; i++) {
        printf("\t");
    }
    printf("--->");
    switch (ast_token_type(ast, token)) {
    case AST_TOKEN_BOP:
        switch (_ast_token_ptr(ast, token)->as.bop) {
        case AST_TOKEN_BOP_EQ:
            printf("= ");
            break;
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
        }
        break;
    case AST_TOKEN_NUM:
        printf("%lf ", _ast_token_ptr(ast, token)->as.num);
        break;
    case AST_TOKEN_PAR_START:
        printf("( ");
        break;
    case AST_TOKEN_PAR_CLOSE:
        printf(") ");
        break;
    case AST_TOKEN_VAR:
        printf("%s ", _ast_token_ptr(ast, token)->as.var);
        break;
    case AST_TOKEN_UOP:
        switch (_ast_token_ptr(ast, token)->as.uop) {
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
    }
    printf("\n");

    _ast_dump_ast_helper(ast, ast_token_left(ast, token), level + 1);
    _ast_dump_ast_helper(ast, ast_token_right(ast, token), level + 1);
}

void ast_dump_ast(ast_t* ast)
{
    _ast_dump_ast_helper(ast, ast->root, 0);
}
