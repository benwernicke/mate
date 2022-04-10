#ifndef AST_H
#define AST_H
#include "error.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef size_t rptr;

typedef enum {
    AST_TOKEN_NUM,       // ben 09.04.22 | numbers
    AST_TOKEN_BOP,       // ben 09.04.22 | binary operations
    AST_TOKEN_UOP,       // ben 09.04.22 | unary operations
    AST_TOKEN_VAR,       // ben 09.04.22 | variables
    AST_TOKEN_PAR_START, // ben 09.04.22 | open parentheses
    AST_TOKEN_PAR_CLOSE, // ben 09.04.22 | close parentheses
} ast_token_type;

typedef enum {
    AST_TOKEN_BOP_ADD, // ben 09.04.22 | + addition
    AST_TOKEN_BOP_SUB, // ben 09.04.22 | - subtraction
    AST_TOKEN_BOP_MUL, // ben 09.04.22 | * multiplication
    AST_TOKEN_BOP_DIV, // ben 09.04.22 | / division
    AST_TOKEN_BOP_EXP, // ben 09.04.22 | ^ exponent
    AST_TOKEN_BOP_EQ,  // ben 10.04.22 | = equals
} ast_token_bop_type;

typedef enum {
    AST_TOKEN_UOP_NEG,  // ben 09.04.22 | -
    AST_TOKEN_UOP_LOG,  // ben 09.04.22 | log() logarithm, base e
    AST_TOKEN_UOP_SIN,  // ben 09.04.22 | sin() sinus
    AST_TOKEN_UOP_COS,  // ben 09.04.22 | cos() cosinus
    AST_TOKEN_UOP_TAN,  // ben 09.04.22 | tan() tangens
    AST_TOKEN_UOP_ASIN, // ben 09.04.22 | asin() arcus sinus
    AST_TOKEN_UOP_ACOS, // ben 09.04.22 | acos() arcus cosinus
    AST_TOKEN_UOP_ATAN, // ben 09.04.22 | atan() arcus tangens
} ast_token_uop_type;

typedef struct ast_token_t ast_token_t;
struct ast_token_t {
    ast_token_type type;
    union {
        double num;
        ast_token_bop_type bop;
        ast_token_uop_type uop;
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

void ast_free(ast_t* ast);
rptr ast_token_right(ast_t* ast, rptr token);
rptr ast_token_left(ast_t* ast, rptr token);

rptr ast_token_add(ast_t* ast, rptr left, rptr right);
rptr ast_token_sub(ast_t* ast, rptr left, rptr right);
rptr ast_token_mul(ast_t* ast, rptr left, rptr right);
rptr ast_token_div(ast_t* ast, rptr left, rptr right);
rptr ast_token_exp(ast_t* ast, rptr left, rptr right);
rptr ast_token_neg(ast_t* ast, rptr arg);
rptr ast_token_log(ast_t* ast, rptr arg);
rptr ast_token_sin(ast_t* ast, rptr arg);
rptr ast_token_cos(ast_t* ast, rptr arg);
rptr ast_token_tan(ast_t* ast, rptr arg);
rptr ast_token_asin(ast_t* ast, rptr arg);
rptr ast_token_acos(ast_t* ast, rptr arg);
rptr ast_token_atan(ast_t* ast, rptr arg);

#endif
