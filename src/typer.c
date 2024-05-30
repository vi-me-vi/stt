// #define  _GNU_SOURCE
#define _POSIX_SOURCE 1
#define  _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "typer.h"


#define KNRM "\x1B[00m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KLEN 5



// Stack structure used for error index tracking
typedef struct int_stack {
    int size;
    int top_index;
    int *items;
} int_stack;


// Helper function to initialize stack
int_stack* stk_init(size_t size) {
    int_stack *st = malloc(sizeof(int_stack));

    st->size = size;
    st->top_index = -1;
    st->items = (int *)malloc(size * sizeof(int));

    return st;
}


// Helper function to properly clean up stack
void stk_free(int_stack *st) {
    free(st->items);
    free(st);
}


int stk_is_full(int_stack *st) {
    return st->top_index == (st->size - 1);
}


int stk_push(int_stack *st, int x) {
    if (stk_is_full(st)) {
        return errno = -1;
    }

    st->top_index += 1;
    st->items[st->top_index] = x;
    return 0;
}


int stk_pop(int_stack *st) {
    if (st->top_index < 0) {
        return errno = -1;
    }

    st->top_index--;
    return st->items[st->top_index+1];
}


// Helper function to check top value on stack without poping
int stk_top(int_stack *st) {
    if (st->top_index < 0) {
        return errno = -1;
    }

    return st->items[st->top_index];
}


// Helper function to remove error markers for indexed error
void error_correction(int_stack *err_st, char **line, ssize_t *len, int *cindex) {
    char *tmp = malloc(sizeof(char) * (*len - (KLEN * 2)));

    // Add text before red marker
    strncpy(tmp, *line, (*cindex - (KLEN * 2)));
    tmp[*cindex - (KLEN * 2)] = '\0';

    // Add char that was marked as error
    sprintf(tmp, "%s%c", tmp, (*line)[*cindex - KLEN]);

    // Add string ending, without additional green marker
    strcat(tmp,  *line + (*cindex + 1));

    // Copy string and allign supporting values
    free(*line);
    *line = tmp;
    *cindex -= KLEN * 2;
    stk_pop(err_st);
    *len -= KLEN * 2;
}


void mark_error(int_stack *err_st, char **line, ssize_t *len, int *cindex) {
    char *tmp = malloc(sizeof(char) * (*len + (KLEN * 2)));

    // Add text before char that has to be marked
    strncpy(tmp, *line, *cindex);
    tmp[*cindex] = '\0';

    // Add red marker
    strcat(tmp, KRED);

    // Add character that has to be marked
    sprintf(tmp, "%s%c", tmp, (*line)[*cindex]);

    // Add green marker for further text hilighting
    strcat(tmp, KGRN);

    // Add text after char that was marked
    strcat(tmp,  *line + (*cindex + 1));

    // Copy string and allign supporting values
    free(*line);
    *line = tmp;
    *cindex += KLEN * 2;
    stk_push(err_st, *cindex);
    *len += KLEN * 2;
}


int run_typer(FILE *fp) {
    int_stack   *err_st;        // Error tracking stack
    int         c;              // Typed-in character
    int         cindex;         // Index in current line
    ssize_t     read;           // Length of current line
    size_t      len   = 0;      // Getline buffer length
    char        *line = NULL;   // Current line

    while ((read = getline(&line, &len, fp)) != -1) {
        cindex = 0;
        err_st = stk_init(read);
        line[strcspn(line, "\n")] = 0;
        read--;

        while (true) {
            fprintf(stdout, "\r%s%.*s%s%s", KGRN, cindex, line, KNRM, &(line[cindex]));
            fflush(stdout);

            c = getchar(); /* Or c = getc(stdin); */

            if (c == '\n') {  // Handle new line
                fprintf(stdout, "\n");
                fflush(stdout);
                break;  // Advance outer loop
            } else if (c == 127) {  // Handle backspace
                if (cindex > 0) {
                    cindex--;

                    // Handle backspace for character marked as error
                    if (cindex == stk_top(err_st)) {
                        error_correction(err_st, &line, &read, &cindex);
                    }
                }
                continue;
            }

            // Handle typing after line is finished
            if (cindex == read) {
                continue;
            }

            // Handle typing error
            if (c != line[cindex]) {
                mark_error(err_st, &line, &read, &cindex);
            }

            fflush(stdout);
            cindex++;

            // Handle <ctrl+c> signal
            if (c == 3) {
                stk_free(err_st);
                if (line) {
                    free(line);
                }
                return 0;
            }
        }
        stk_free(err_st);
    }

    if (line) {
        free(line);
    }

    return 0;
}
