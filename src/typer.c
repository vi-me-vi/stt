#define  _GNU_SOURCE
// #define _POSIX_SOURCE 1
// #define  _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "typer.h"


const char *KNRM = "\x1B[00m";
const char *KRED = "\x1B[31m";
const char *KGRN = "\x1B[32m";
const char *KYEL = "\x1B[33m";
const char *KPUR = "\x1B[35m";
const char *KREV = "\x1B[07m";
const char *KUND = "\x1B[04m";
const int  KLEN  = 5;



/* Helper function to remove error markers for indexed error */
void error_correction(char **line, ssize_t *len, int *cindex) {
    char *tmp = calloc(*len - (KLEN * 2) + 1, sizeof(char));

    /* Add text before red marker */
    strncpy(tmp, *line, (*cindex - (KLEN * 2)));
    tmp[*cindex - (KLEN * 2)] = '\0';

    /* Add char that was marked as error */
    strncat(tmp, &((*line)[*cindex - KLEN]), 1);

    /* Add string ending, without additional green marker */
    strcat(tmp,  *line + (*cindex + 1));

    /* Copy string and allign supporting values */
    *line = realloc(*line, (*len - (KLEN * 2) + 1) * sizeof(char));
    if (line != NULL) {
        memcpy(*line, tmp, sizeof(char) * (*len - (KLEN * 2) + 1));
    }
    *cindex -= KLEN * 2;
    *len -= KLEN * 2;
    free(tmp);
}


void mark_error(char **line, ssize_t *len, int *cindex, const char* error_marker, const char* correct_marker) {
    char *tmp = calloc(*len + (KLEN * 2) + 1, sizeof(char));

    /* Add text before char that has to be marked */
    strncpy(tmp, *line, *cindex);
    tmp[*cindex] = '\0';

    /* Add red marker */
    strcat(tmp, error_marker);

    /* Add character that has to be marked */
    strncat(tmp, *line + *cindex, 1);

    /* Add green marker for further text hilighting */
    strcat(tmp, correct_marker);

    /* Add text after char that was marked */
    strcat(tmp, *line + *cindex + 1);

    /* Copy string and allign supporting values */
    *line = realloc(*line, (*len + (KLEN * 2) + 1) * sizeof(char));
    if (line != NULL) {
        memcpy(*line, tmp, sizeof(char) * (*len + (KLEN * 2) + 1));
    }
    *cindex += KLEN * 2;
    *len += KLEN * 2;
    free(tmp);
}


int run_typer(FILE *fp, bool preserve_formatting, bool monochrome_mode) {
    int         c             = 0;                                  /* Typed-in character */
    int         cindex        = 0;                                  /* Index in current line */
    int         err_count     = 0;                                  /* Error counter */
    ssize_t     read          = 0;                                  /* Length of current line */
    time_t      start         = 0;                                  /* Clock used to measure timings */
    size_t      len           = 0;                                  /* Getline buffer length */
    char        *line         = NULL;                               /* Current line */
    char        *work_line    = NULL;                               /* Current line coppied for modification */
    char        *start_marker = NULL;                               /* Used to cut off padded strings */
    int         typed_chars   = 0;                                  /* Tracking number of chars typed-in */
    int         sum_acc       = 0;                                  /* Sum of accuracies used to calculate average */
    float       sum_speed     = 0;                                  /* Sum of speeds to calculate average */
    int         line_c        = 0;                                  /* Counter of lines user interracted with ti show averages */
    const char  *correct_m    = (monochrome_mode) ? KUND : KGRN;    /* Marker used to hilight correct input */
    const char  *incorrect_m  = (monochrome_mode) ? KREV : KRED;    /* Marker used to hilight incorrect input */
    const char  *stat_c       = (monochrome_mode) ? "" : KPUR;      /* Used to hilight stats printout */

    fprintf(stdout, "Press <ctrl+c> to stop typer\n");
    fflush(stdout);

    while ((read = getline(&line, &len, fp)) != -1) {

        /* Handle empty strings and cut off padding */
        start_marker = line;
        if(line[0] == '\n') {
            continue;
        } else if(!preserve_formatting && (line[0] == ' ' || line[0] == '\t')) {
            while (*start_marker == ' ' || *start_marker == '\t') {
                start_marker++;
                read--;
            }
        }
        work_line = strdup(start_marker);
        work_line[read-1] = '\0';
        read--;

        /* Pseudo-stack used for keeping error indeces */
        int stk_top = -1;
        int err_stk[read];

        cindex = 0;
        err_count = 0;
        typed_chars = 0;
        start = 0;

        while (true) {
            fprintf(stdout, "\r%s%.*s%s%s", correct_m, cindex, work_line, KNRM, &(work_line[cindex]));
            fflush(stdout);

            c = getchar(); /* Or c = getc(stdin); */

            /* When started typing at new line (ignoring backspace), start timer */
            if (start == 0 && (c != '\n' && c != 127)) {
                time(&start);
            }

            if (c == '\n') {  /* Handle new line */
                if (start != 0) {  /* Display stats */
                    time_t end;
                    time(&end);

                    int acc = (int)(100.0 - ((float)err_count / (float)typed_chars * 100.0));
                    float speed = (float)cindex / difftime(end, start);

                    fprintf(
                        stdout, "    %s[ Accuracy: %d%% | Speed: %.2lf char/s ]%s\n",
                        stat_c, acc, speed, KNRM
                    );
                    fflush(stdout);

                    sum_acc += acc;
                    sum_speed += speed;
                    line_c++;
                } else { /* Handle ignored new line */
                    fprintf(stdout, "\n");
                    fflush(stdout);
                }

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
                mark_error(&work_line, &read, &cindex, incorrect_m, correct_m);
                err_count++;
                stk_top++;
                err_stk[stk_top] = cindex;
            }

            cindex++;
            typed_chars++;

            /* Handle <ctrl+c> signal */
            if (c == 3) {
                if (line_c > 1) {
                    fprintf(
                        stdout, "\n\n%s[ Average Accuracy: %d%% | Average Speed %.2lf char/s ]%s\n",
                        stat_c, sum_acc / line_c, sum_speed / (float)line_c, KNRM
                    );
                    fflush(stdout);
                } else {
                    fprintf(stdout, "\n");
                    fflush(stdout);
                }
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
    if (line_c > 1) {
        fprintf(
            stdout, "\n%s[ Average Accuracy: %d%% | Average Speed %.2lf char/s ]%s\n",
            stat_c, sum_acc / line_c, sum_speed / (float)line_c, KNRM
        );
        fflush(stdout);
    }
    return 0;
}

