// TODO: move this in seperate file -> use one file for diff code one file for integration code one file for simplification...
// --> but one common header
#include "diff.h"
#include "ast.h"



void token_l_set(expr_buf_t* eb, expr_t* token, expr_t* target)
{
    token->l = (target != NULL) ? target - eb->buf : -1;
}

void token_r_set(expr_buf_t* eb, expr_t* token, expr_t* target)
{
    token->r = (target != NULL) ? target - eb->buf : -1;
}

void token_up_set(expr_buf_t* eb, expr_t* token, expr_t* target)
{
    token->up = (target != NULL) ? target - eb->buf : -1;
}

expr_buf_t* eb_cons(size_t init_scope)
{
    expr_buf_t* eb = (expr_buf_t*)malloc(sizeof(*eb));
    eb->scope = init_scope;
    eb->buf = malloc(init_scope * sizeof(*eb->buf));
    return eb;
}

void eb_free(expr_buf_t* eb)
{
    if (eb) {
        free(eb->buf);
        free(eb);
    }
}

void lexer_add_bop(expr_buf_t* eb, bop_type type)
{
    expr_t* token = eb_more(eb);
    token->type = EXPR_BOP;
    token->as.bop = type;
}

void lexer_add_num(expr_buf_t* eb, char** sp)
{
    expr_t* token = eb_more(eb);
    char* s = *sp;
    char* ss = s;
    char tmp;
    while (*ss && (isdigit(*ss) || *ss == '.')) {
        ss++;
    }
    tmp = *ss;
    *ss = 0;
    token->type = EXPR_NUM;
    token->as.num = atof(s);
    *ss = tmp;
    *sp = ss - 1;
}

void lexer_add_var(expr_buf_t* eb, char** sp)
{
    expr_t* token = eb_more(eb);
    token->type = EXPR_VAR;
    char* s = *sp;
    char* ss = s;
    char tmp;

    while (*ss && (isdigit(*ss) || isalpha(*ss) || *ss == '_')) {
        ss++;
    }
    tmp = *ss;
    *ss = 0;
    strncpy(token->as.var, s, 16);
    *ss = tmp;
    *sp = ss - 1;
}

void lexer_add_simple(expr_buf_t* eb, expr_type type)
{
    expr_t* token = eb_more(eb);
    token->type = type;
}

void lexer(expr_buf_t* eb, char* s)
{
    while (*s) {
        if (*s == '+') {
            lexer_add_bop(eb, BOP_ADD);
        } else if (*s == '-') {
            lexer_add_bop(eb, BOP_SUB);
        } else if (*s == '*') {
            lexer_add_bop(eb, BOP_MUL);
        } else if (*s == '/') {
            lexer_add_bop(eb, BOP_DIV);
        } else if (*s == '^') {
            lexer_add_bop(eb, BOP_EXP);
        } else if (isdigit(*s)) {
            lexer_add_num(eb, &s);
        } else if (isalpha(*s) || *s == '_') {
            lexer_add_var(eb, &s);
        } else if (*s == '(') {
            lexer_add_simple(eb, EXPR_POP);
        } else if (*s == ')') {
            lexer_add_simple(eb, EXPR_PCL);
        }
        s++;
    }
}

void dump_token(expr_t* token)
{
    switch (token->type) {
    case EXPR_LN:
        printf("ln ");
        break;
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
    }
}

void eb_dump(expr_buf_t* eb)
{
    expr_t* it = eb->buf;
    while (it < eb->buf + eb->size) {
        dump_token(it);
        it++;
    }
    printf("\n");
}

#define EXPR_NUM_PREC 4
int ast_token_prec(expr_t* token)
{
    switch (token->type) {
    case EXPR_BOP:
        switch (token->as.bop) {
        case BOP_ADD:
        case BOP_SUB:
            return 1;
        case BOP_MUL:
        case BOP_DIV:
            return 2;
        case BOP_EXP:
            return 3;
        }
        break;
    case EXPR_POP:
        return token->as.pop;
        break;
    case EXPR_PCL:
        assert(0 && "you should not get here");
        break;
    case EXPR_NUM:
    case EXPR_LN: // TODO: think about unary functions
    case EXPR_VAR:
        return EXPR_NUM_PREC;
    }
    assert(0 && "you should not get here");
}

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

expr_buf_t* ast_from_str(char* s)
{
    expr_buf_t* eb = eb_cons(2);
    lexer(eb, s);
    build_ast(eb);
    eb->root = ast_root(eb, eb->buf);
    return eb;
}

void ast_printf(expr_buf_t* eb, expr_t* token)
{
    expr_t* l = token_l(eb, token);
    expr_t* r = token_r(eb, token);

    if (token == NULL) {
        return;
    }

    if (l != NULL) {
        if (ast_token_prec(l) < ast_token_prec(token)) {
            printf("( ");
            ast_printf(eb, l);
            printf(") ");
        } else {
            ast_printf(eb, l);
        }
    }

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
    case EXPR_LN:
        printf("ln ");
        break;
    case EXPR_NUM:
        printf("%lf ", token->as.num);
        break;
    case EXPR_POP:
    case EXPR_PCL:
        break;
    case EXPR_VAR:
        printf("%s ", token->as.var);
        break;
    }

    if (r != NULL) {
        if (ast_token_prec(r) < ast_token_prec(token)) {
            printf("( ");
            ast_printf(eb, r);
            printf(") ");
        } else {
            ast_printf(eb, r);
        }
    }
}

rptr _diff_copy_tree(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    if (from_token == NULL) {
        return -1;
    }

    rptr to_token = eb_rmore(to);
    *eb_synth(to, to_token) = *from_token;
    rptr l = _diff_copy_tree(from, to, token_l(from, from_token));
    rptr r = _diff_copy_tree(from, to, token_r(from, from_token));

    if (l != -1) {
        token_l_set(to, eb_synth(to, to_token), eb_synth(to, l));
    }

    if (r != -1) {
        token_r_set(to, eb_synth(to, to_token), eb_synth(to, r));
    }

    return to_token;
}

bool istagged(expr_buf_t* eb, expr_t* token)
{
    return token && (token->type == EXPR_VAR || 
            istagged(eb, token_l(eb, token)) || 
            istagged(eb, token_r(eb, token)));
}

rptr diff(expr_buf_t* from, expr_buf_t* to, expr_t* from_token);


void token_l_rel_set(expr_buf_t* eb, rptr t, rptr l)
{
    token_l_set(eb, eb_synth(eb, t), eb_synth(eb, l));
}

void token_r_rel_set(expr_buf_t* eb, rptr t, rptr r)
{
    token_r_set(eb, eb_synth(eb, t), eb_synth(eb, r));
}

rptr token_lr_rel_set(expr_buf_t* to, rptr t, rptr l, rptr r)
{
    token_l_rel_set(to, t, l);
    token_r_rel_set(to, t, r);
    return t;
}

rptr token_rel_bop(expr_buf_t* eb, bop_type type, rptr l, rptr r)
{
    rptr t = eb_rmore(eb);
    eb_synth(eb, t)->type = EXPR_BOP;
    eb_synth(eb, t)->as.bop = type;
    token_lr_rel_set(eb, t, l, r);
    return t;
}

rptr token_rel_add(expr_buf_t* eb, rptr l, rptr r)
{
    return token_rel_bop(eb, BOP_ADD, l, r);
}

rptr token_rel_mul(expr_buf_t* eb, rptr l, rptr r)
{
    return token_rel_bop(eb, BOP_MUL, l, r);
}

rptr token_rel_div(expr_buf_t* eb, rptr l, rptr r)
{
    return token_rel_bop(eb, BOP_DIV, l, r);
}

rptr token_rel_sub(expr_buf_t* eb, rptr l, rptr r)
{
    return token_rel_bop(eb, BOP_SUB, l, r);
}

rptr token_rel_exp(expr_buf_t* eb, rptr l, rptr r)
{
    return token_rel_bop(eb, BOP_EXP, l, r);
}

rptr token_rel_num(expr_buf_t* eb, double num)
{
    rptr t = eb_rmore(eb);
    eb_synth(eb, t)->type = EXPR_NUM;
    eb_synth(eb, t)->as.num = num;
    return t;
}

rptr token_rel_ln(expr_buf_t* eb, rptr r)
{
    rptr t = eb_rmore(eb);
    eb_synth(eb, t)->type = EXPR_LN;
    token_r_set(eb, eb_synth(eb, t), eb_synth(eb, r));
    return t;
}

rptr _diff_constant(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    return token_rel_num(to, 0);
}

rptr _diff_add(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    return token_rel_add(to,
            diff(from, to, token_l(from, from_token)),
            diff(from, to, token_r(from, from_token))
            );
}

rptr _diff_sub(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    return token_rel_sub(to,
            diff(from, to, token_l(from, from_token)),
            diff(from, to, token_r(from, from_token))
            );
}

rptr _diff_mul(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    return token_rel_add(to,
            token_rel_mul(to,
                _diff_copy_tree(from, to, token_l(from, from_token)),
                diff(from, to, token_r(from, from_token))
                ),
            token_rel_mul(to,
                diff(from, to, token_l(from, from_token)),
                _diff_copy_tree(from, to, token_r(from, from_token))
                )
            );
}

rptr _diff_div(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    return token_rel_div(to,
            token_rel_sub(to,
                token_rel_mul(to,
                    diff(from, to, token_l(from, from_token)),
                    _diff_copy_tree(from, to, token_r(from, from_token))
                    ),
                token_rel_mul(to,
                    _diff_copy_tree(from, to, token_l(from, from_token)),
                    diff(from, to, token_r(from, from_token))
                    )
                ),
            token_rel_exp(to,
                _diff_copy_tree(from, to, token_r(from, from_token)),
                token_rel_num(to, 2)
                )
            );
}

rptr _diff_exp(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    return token_rel_mul(to,
            _diff_copy_tree(from, to, from_token),
            token_rel_add(to,
                token_rel_div(to,
                    token_rel_mul(to,
                        diff(from, to, token_l(from, from_token)),
                        _diff_copy_tree(from, to, token_r(from, from_token))
                        ),
                    _diff_copy_tree(from, to, token_l(from, from_token))
                    ),
                token_rel_mul(to,
                    token_rel_ln(to, _diff_copy_tree(from, to, token_l(from, from_token))),
                    diff(from, to, token_r(from, from_token))
                    )
                )
            );
}

rptr _diff_var(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    return token_rel_num(to, 1);
}

rptr _diff_ln(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    return token_rel_div(to,
            diff(from, to, token_r(from, from_token)),
            _diff_copy_tree(from, to, token_r(from, from_token))
            );
}

rptr diff(expr_buf_t* from, expr_buf_t* to, expr_t* from_token)
{
    if (!istagged(from, from_token)) {
        return _diff_constant(from, to, from_token);
    }
    switch (from_token->type) {
    case EXPR_BOP:
        switch (from_token->as.bop) {
        case BOP_ADD:
            return _diff_add(from, to, from_token);
        case BOP_SUB:
            return _diff_sub(from, to, from_token);
        case BOP_MUL:
            return _diff_mul(from, to, from_token);
        case BOP_DIV:
            return _diff_div(from, to, from_token);
        case BOP_EXP:
            return _diff_exp(from, to, from_token);
        }
    case EXPR_LN:
        return _diff_ln(from, to, from_token);
    case EXPR_NUM:
        assert(0 && "you should not get here"); // will be done by !istagged on top
        break;
    case EXPR_POP:
        return diff(from, to, token_l(from, from_token));
    case EXPR_PCL:
        break;
    case EXPR_VAR:
        return _diff_var(from, to, from_token);
    }
    assert(0 && "you should not get here");
}

void ast_connect_up(expr_buf_t* eb, expr_t* last, expr_t* token)
{
    if (token == NULL) {
        return;
    }
    token_up_set(eb, token, last);
    ast_connect_up(eb, token, token_l(eb, token));
    ast_connect_up(eb, token, token_r(eb, token));
}

void token_num_set(expr_buf_t* eb, expr_t* token, double num )
{
    token->type = EXPR_NUM;
    token->as.num = num;
    token->l = token->r = -1;
}

bool _simplefy_add(expr_buf_t* eb, expr_t* token)
{
    if (token_l(eb, token)->type == EXPR_NUM && (int)token_l(eb, token)->as.num == 0) {
        *token = *token_r(eb, token);
        return 1;
    } else if (token_r(eb, token)->type == EXPR_NUM && (int)token_r(eb, token)->as.num == 0) {
        *token = *token_l(eb, token);
        return 1;
    }
    return 0;
}

bool _simplefy_sub(expr_buf_t* eb, expr_t* token)
{
    if (token_r(eb, token)->type == EXPR_NUM && (int)token_r(eb, token)->as.num == 0) {
        *token = *token_l(eb, token);
        return 1;
    }
    return 0;
}

bool _simplefy_div(expr_buf_t* eb, expr_t* token)
{
    if (token_l(eb, token)->type == EXPR_NUM && (int)token_l(eb, token)->as.num == 0) {
        token_num_set(eb, token, 0);
        return 1;
    } else if (token_r(eb, token)->type == EXPR_NUM && (int)token_r(eb, token)->as.num == 1) {
        *token = *token_l(eb, token);
        return 1;
    }
    // TODO a / a
    return 0;
}

bool _simplefy_mul(expr_buf_t* eb, expr_t* token)
{
    if ((token_l(eb, token)->type == EXPR_NUM && (int)token_l(eb, token)->as.num == 0) || 
        (token_r(eb, token)->type == EXPR_NUM && (int)token_r(eb, token)->as.num == 0)) {
        token_num_set(eb, token, 0);
        return 1;
    } else if (token_l(eb, token)->type == EXPR_NUM && (int)token_l(eb, token)->as.num == 1) {
        *token = *token_r(eb, token);
        return 1;
    } else if (token_r(eb, token)->type == EXPR_NUM && (int)token_r(eb, token)->as.num == 1) {
        *token = *token_l(eb, token);
        return 1;
    }
    return 0;
}


bool _simplefy_exp(expr_buf_t* eb, expr_t* token)
{
    if (token_l(eb, token)->type == EXPR_NUM && (int)token_l(eb, token)->as.num == 0) {
        token_num_set(eb, token, 0);
        return 1;
    } else if (token_r(eb, token)->type == EXPR_NUM && (int)token_r(eb, token)->as.num == 0) {
        token_num_set(eb, token, 1);
        return 1;
    } else if (token_l(eb, token)->type == EXPR_NUM && (int)token_l(eb, token)->as.num == 1) {
        token_num_set(eb, token, 1);
        return 1;
    } else if (token_r(eb, token)->type == EXPR_NUM && (int)token_r(eb, token)->as.num == 1) {
        *token = *token_l(eb, token);
        return 1;
    }
    return 0;
}

bool _ast_simplefy_try_calc(expr_buf_t* eb, expr_t* token)
{
    if(token_l(eb, token)->type == EXPR_NUM && token_r(eb, token)->type == EXPR_NUM)
    {
        switch(token->as.bop) {
            case BOP_ADD:
                token->as.num = token_l(eb, token)->as.num + token_r(eb, token)->as.num;
                break;
            case BOP_SUB:
                token->as.num = token_l(eb, token)->as.num - token_r(eb, token)->as.num;
                break;
            case BOP_MUL:
                token->as.num = token_l(eb, token)->as.num * token_r(eb, token)->as.num;
                break;
            case BOP_DIV:
                token->as.num = token_l(eb, token)->as.num / token_r(eb, token)->as.num;
                break;
            case BOP_EXP:
                token->as.num = pow(token_l(eb, token)->as.num, token_r(eb, token)->as.num);
                break;
        }
        token->type = EXPR_NUM;
        token->l = token->r = -1;
        return 1;
    }
    return 0;
}

bool _ast_simplefy_pass(expr_buf_t* eb, expr_t* token)
{
    if (token == NULL) {
        return 0;
    }
    bool change = 0;
    if (token->type == EXPR_BOP) {
        assert(token_l(eb, token) && token_r(eb, token));
        change = _ast_simplefy_try_calc(eb, token);         //may invalidate l and r -> if needed
        if(!change) {
            switch (token->as.bop) {
            case BOP_ADD:
                change = _simplefy_add(eb, token);
                break;
            case BOP_SUB:
                change = _simplefy_sub(eb, token);
                break;
            case BOP_MUL:
                change = _simplefy_mul(eb, token);
                break;
            case BOP_DIV:
                change = _simplefy_div(eb, token);
                break;
            case BOP_EXP:
                change = _simplefy_div(eb, token);
                break;
            }
        }
    }
    return change || _ast_simplefy_pass(eb, token_l(eb, token)) || _ast_simplefy_pass(eb, token_r(eb, token));
}

void ast_simplefy(expr_buf_t* eb, expr_t* token)
{
    bool change;
    while ((change = _ast_simplefy_pass(eb, token)));
}

int main(void)
{
    char input[] = "(x + 1) ^2";
    expr_buf_t* eb = ast_from_str(input);

    expr_buf_t* diffed = eb_cons(eb->size);
    diffed->root = eb_synth(diffed, diff(eb, diffed, eb->root));

    ast_connect_up(diffed, 0, diffed->root);
    ast_printf(diffed, diffed->root);
    printf("\n");

    ast_simplefy(diffed, diffed->root);
    ast_printf(diffed, diffed->root);
    printf("\n");

    eb_free(eb);
    eb_free(diffed);
    return 0;
}
