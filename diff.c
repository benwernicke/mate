#include "ast.h"

expr_t* build_ast_connect_two(expr_buf_t* eb, expr_t* last, expr_t* curr)
{
    // if closing parentheses move up till matching opening parentheses
    if (curr->type == EXPR_PCL) {
        while (!(last->type == EXPR_POP && last->as.pop != EXPR_NUM_PREC)) {
            last = token_up(eb, last);
        }
        // TODO: remove POP token not necessary any more -> watch out for last->up == NULL
        last->as.pop = EXPR_NUM_PREC;
        return last;
    }

    expr_t* llast = NULL;
    while (last != NULL && ast_token_prec(last) >= ast_token_prec(curr)) {
        llast = last;
        last = token_up(eb, last);
    }
    if (last) {
        if (token_l(eb, last) == llast) {
            token_l_set(eb, last, curr);
        } else {
            token_r_set(eb, last, curr);
        }
    }
    token_l_set(eb, curr, llast);
    token_up_set(eb, curr, last);
    if (llast != NULL) {
        token_up_set(eb, llast, curr);
    }
    return curr;
}

void dump_ast(expr_buf_t* eb, expr_t* token, int level)
{
    if (!token) {
        return;
    }
    int i;
    for (i = 0; i < level; i++) {
        printf("\t");
    }
    printf("--->");
    switch (token->type) {
    case EXPR_BOP:
        switch (token->as.bop) {
        case BOP_ADD:
            printf("+ ");
            break;
        case BOP_SUB:
            printf("- ");
            break;
        case BOP_MUL:
            printf("* ");
            break;
        case BOP_DIV:
            printf("/ ");
            break;
        case BOP_EXP:
            printf("^ ");
            break;
        }
        break;
    case EXPR_NUM:
        printf("%lf ", token->as.num);
        break;
    case EXPR_POP:
        printf("( ");
        break;
    case EXPR_PCL:
        printf(") ");
        break;
    case EXPR_VAR:
        printf("%s ", token->as.var);
        break;
    case EXPR_LN:
        printf("ln ");
        break;
    }
    printf("\n");

    dump_ast(eb, token_l(eb, token), level + 1);
    dump_ast(eb, token_r(eb, token), level + 1);
}

expr_t* ast_root(expr_buf_t* eb, expr_t* token)
{
    if (token_up(eb, token) == NULL) {
        return token;
    }
    return ast_root(eb, token_up(eb, token));
}

void build_ast(expr_buf_t* eb)
{
    expr_t* last = eb->buf;
    expr_t* curr = eb->buf + 1;

    while (curr < eb->buf + eb->size) {
        if (curr->type == EXPR_POP) {
            curr->as.pop = EXPR_NUM_PREC;
            last = build_ast_connect_two(eb, last, curr);
            curr->as.pop = 0;
        } else {
            last = build_ast_connect_two(eb, last, curr);
        }
        curr++;
    }
}
