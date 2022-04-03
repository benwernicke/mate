#ifndef AST_H
#define AST_H

#include <stdlib.h>

typedef enum {
    EXPR_BOP,
    EXPR_NUM,
    EXPR_VAR,
    EXPR_POP,
    EXPR_PCL,
    EXPR_LN,
} expr_type;

typedef enum {
    BOP_ADD,
    BOP_SUB,
    BOP_MUL,
    BOP_DIV,
    BOP_EXP,
} bop_type;

typedef size_t rptr;

typedef struct expr_t expr_t;
struct expr_t {
    expr_type type;
    union {
        double num;
        bop_type bop;
        char var[16];
        int pop;
    } as;
    rptr l;
    rptr r;
    rptr up;
};

typedef struct ast_t ast_t;
struct ast_t {
    size_t alloced;
    size_t used;
    expr_t* buf;
    rptr root;
};
#endif
