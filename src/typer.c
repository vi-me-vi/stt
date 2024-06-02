#define  _GNU_SOURCE
// #define _POSIX_SOURCE 1
// #define  _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "typer.h"


#define KNRM "\x1B[00m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KPUR "\x1B[35m"
#define KLEN 5



/* Helper function to remove error markers for indexed error */
void error_correction(char **line, ssize_t *len, int *cindex) {
    char *tmp = calloc(*len - (KLEN * 2) + 1, sizeof(char));

    /* Add text before red marker */
    strncpy(tmp, *line, (*cindex - (KLEN * 2)));
    tmp[*cindex - (KLEN * 2)] = '\0';

    /* Add char that was marked as error */
    strncat(tmp, &((*line)[*cindex - KLEN]), 1);;

    /* Add string ending, without additional green marker */
    strcat(tmp,  *line + (*cindex + 1));

    /* Copy string and allign supporting values */
    *line = calloc(*len - (KLEN * 2) + 1, sizeof(char));
    if (line != NULL) {
        memcpy(*line, tmp, sizeof(char) * (*len - (KLEN * 2) + 1));
    }
    *cindex -= KLEN * 2;
    *len -= KLEN * 2;
    free(tmp);
}


void mark_error(char **line, ssize_t *len, int *cindex) {
    char *tmp = calloc(*len + (KLEN * 2) + 1, sizeof(char));

    /* Add text before char that has to be marked */
    strncpy(tmp, *line, *cindex);
    tmp[*cindex] = '\0';

    /* Add red marker */
    strcat(tmp, KRED);

    /* Add character that has to be marked */
    strncat(tmp, *line + *cindex, 1);

    /* Add green marker for further text hilighting */
    strcat(tmp, KGRN);

    /* Add text after char that was marked */
    strcat(tmp, *line + *cindex + 1);

    /* Copy string and allign supporting values */
    *line = calloc(*len + (KLEN * 2) + 1, sizeof(char));
    if (line != NULL) {
        memcpy(*line, tmp, sizeof(char) * (*len + (KLEN * 2) + 1));
    }
    *cindex += KLEN * 2;
    *len += KLEN * 2;
    free(tmp);
}


int run_typer(FILE *fp) {
    int         c;                      // Typed-in character
    int         cindex;                 // Index in current line
    int         err_count;              // Error counter
    ssize_t     read;                   // Length of current line
    time_t      start         = 0;      // Clock used to measure timings
    size_t      len           = 0;      // Getline buffer length
    char        *line         = NULL;   // Current line
    char        *work_line    = NULL;   // Current line coppied for modification
    char        *start_marker = NULL;   // Used to cut off padded strings

    fprintf(stdout, "Press <ctrl+c> to stop typer\n");
    fflush(stdout);

    while ((read = getline(&line, &len, fp)) != -1) {

        /* Handle empty strings and cut off padding */
        start_marker = line;
        if(line[0] == '\n') {
            continue;
        } else if(line[0] == ' ' || line[0] == '\t') {
            while (*start_marker == ' ' || *start_marker == '\t') {
                start_marker++;
                read--;
            }
        }
        work_line = strdup(start_marker);
        /*
         * // Method used before string padding removal was added
         * work_line = calloc(read, sizeof(char));
         * if (work_line && line) {
         *    memcpy(work_line, line, read);
         * }
        */
        work_line[read-1] = '\0';
        read--;

        /* Pseudo-stack used for keeping error indeces */
        int stk_top = -1;
        int err_stk[read];

        cindex = 0;
        err_count = 0;
        start = 0;

        while (true) {
            fprintf(stdout, "\r%s%.*s%s%s", KGRN, cindex, work_line, KNRM, &(work_line[cindex]));
            fflush(stdout);

            c = getchar(); /* Or c = getc(stdin); */

            /* When started typing at new line, start timer */
            if (start == 0) {
                time(&start);
            }

            if (c == '\n') {  /* Handle new line */
                time_t end;
                time(&end);
                fprintf(
                    stdout, "    %s[ Accuracy: %d%% | Speed: %.2lf char/s ]%s\n",
                    KPUR,
                    (int)(100.0 - ((float)err_count / (float)read * 100.0)),
                    (float)cindex / difftime(end, start),
                    KNRM
                );
                fflush(stdout);
                break;  /* Advance outer loop */
            } else if (c == 127) {  /* Handle backspace */
                if (cindex > 0) {
                    cindex--;

                    /* Handle backspace for character marked as error */
                    if (stk_top > -1 && cindex == err_stk[stk_top]) {
                        error_correction(&work_line, &read, &cindex);
                        stk_top--;
                    }
                }
                continue;
            }

            /* Handle typing after line is finished */
            if (cindex == read) {
                continue;
            }

            /* Handle typing error */
            if (c != work_line[cindex]) {
                mark_error(&work_line, &read, &cindex);
                err_count++;
                stk_top++;
                err_stk[stk_top] = cindex;
            }

            cindex++;

            /* Handle <ctrl+c> signal */
            if (c == 3) {
                if (work_line) {
                    free(work_line);
                    free(line);
                }
                return 0;
            }
        }
        free(work_line);
    }

    if (line) {
        free(line);
    }
    return 0;
}

