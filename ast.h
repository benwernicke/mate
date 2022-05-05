#ifndef AST_H
#define AST_H
#include "error.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef size_t rptr;

typedef enum {
    AST_TOKEN_NUM, // ben 09.04.22 | numbers
    AST_TOKEN_BOP, // ben 09.04.22 | binary operations
    AST_TOKEN_UOP, // ben 09.04.22 | unary operations
    AST_TOKEN_VAR, // ben 09.04.22 | variables
} ast_token_type_t;

typedef enum {
    AST_TOKEN_BOP_ADD, // ben 09.04.22 | + addition
    AST_TOKEN_BOP_SUB, // ben 09.04.22 | - subtraction
    AST_TOKEN_BOP_MUL, // ben 09.04.22 | * multiplication
    AST_TOKEN_BOP_DIV, // ben 09.04.22 | / division
    AST_TOKEN_BOP_EXP, // ben 09.04.22 | ^ exponent
    AST_TOKEN_BOP_EQ,  // ben 10.04.22 | = equals
} ast_token_bop_type_t;

typedef enum {
    AST_TOKEN_UOP_NEG,  // ben 09.04.22 | -
    AST_TOKEN_UOP_LOG,  // ben 09.04.22 | log() logarithm, base e
    AST_TOKEN_UOP_SIN,  // ben 09.04.22 | sin() sinus
    AST_TOKEN_UOP_COS,  // ben 09.04.22 | cos() cosinus
    AST_TOKEN_UOP_TAN,  // ben 09.04.22 | tan() tangens
    AST_TOKEN_UOP_ASIN, // ben 09.04.22 | asin() arcus sinus
    AST_TOKEN_UOP_ACOS, // ben 09.04.22 | acos() arcus cosinus
    AST_TOKEN_UOP_ATAN, // ben 09.04.22 | atan() arcus tangens
} ast_token_uop_type_t;

typedef struct ast_token_t ast_token_t;
struct ast_token_t {
    ast_token_type_t type;
    union {
        double num;
        ast_token_bop_type_t bop;
        ast_token_uop_type_t uop;
        char var[16];
    } as;
    rptr left;
    rptr right;
};

typedef struct ast_t ast_t;
struct ast_t {
    size_t allocated_tokens;
    size_t used_tokens;
    rptr root;
    ast_token_t* buf;
};

ast_t* ast_cons(void);
void ast_free(ast_t* ast);

bool ast_token_is_token_null(ast_t* ast, rptr token);
bool ast_token_is_left_null(ast_t* ast, rptr token);
bool ast_token_is_right_null(ast_t* ast, rptr token);

rptr ast_token_left(ast_t* ast, rptr token);
rptr ast_token_right(ast_t* ast, rptr token);

void ast_token_left_set(ast_t* ast, rptr token, rptr val);
void ast_token_right_set(ast_t* ast, rptr token, rptr val);

rptr ast_token_add_cons(ast_t* ast, rptr left, rptr right);
rptr ast_token_sub_cons(ast_t* ast, rptr left, rptr right);
rptr ast_token_mul_cons(ast_t* ast, rptr left, rptr right);
rptr ast_token_div_cons(ast_t* ast, rptr left, rptr right);
rptr ast_token_exp_cons(ast_t* ast, rptr left, rptr right);
rptr ast_token_neg_cons(ast_t* ast, rptr arg);
rptr ast_token_log_cons(ast_t* ast, rptr arg);
rptr ast_token_sin_cons(ast_t* ast, rptr arg);
rptr ast_token_cos_cons(ast_t* ast, rptr arg);
rptr ast_token_tan_cons(ast_t* ast, rptr arg);
rptr ast_token_asin_cons(ast_t* ast, rptr arg);
rptr ast_token_acos_cons(ast_t* ast, rptr arg);
rptr ast_token_atan_cons(ast_t* ast, rptr arg);
rptr ast_token_num_cons(ast_t* ast, double num);
rptr ast_token_var_cons(ast_t* ast, char* s);

ast_token_type_t ast_token_type(ast_t* ast, rptr token);

void ast_token_print(ast_t* ast, rptr token);

void ast_connect_up(ast_t* ast);

ast_t* ast_ast_from_str(char* s);
void ast_print_buf(ast_t* ast);
void ast_dump_ast(ast_t* ast);

#endif
