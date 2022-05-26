#ifndef ERR_H
#define ERR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define err_error_if(cond, ...)                          \
    {                                                    \
        if (cond) {                                      \
            fprintf(stderr, "\033[0;31mError: \033[0m"); \
            fprintf(stderr, __VA_ARGS__);                \
            fprintf(stderr, "\n");                       \
            exit(1);                                     \
        }                                                \
    }

#define err_error(...)                               \
    {                                                \
        fprintf(stderr, "\033[0;31mError: \033[0m"); \
        fprintf(stderr, __VA_ARGS__);                \
        fprintf(stderr, "\n");                       \
        exit(1);                                     \
    }

#endif
