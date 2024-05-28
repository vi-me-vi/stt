// #define  _GNU_SOURCE
#define _POSIX_SOURCE 1
#define  _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "term.h"


#define KNRM  "\x1B[00m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"


int main(void) {
    int c, cindex;
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    SttTerm term = {
        .stream = stdin,
        .save_state = (struct termios *) malloc(sizeof(struct termios))
    };

    /* Make terminal at standard input unbuffered and raw. */
    if (term_init(&term)) {
        fprintf(stderr, "Cannot make standard input raw: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Press <ctrl+c> to quit.\n");
    fflush(stdout);

    fp = fopen("test.txt", "r");
    if (!fp) {
        return EXIT_FAILURE;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        cindex = 0;
        line[strcspn(line, "\n")] = 0;

        while (true) {
            fprintf(stdout, "\r%s%.*s%s%s", KGRN, cindex, line, KNRM, &(line[cindex]));
            fflush(stdout);
            c = getchar(); /* Or c = getc(stdin); */

            if (c == '\n') {
                fprintf(stdout, "\n");
                fflush(stdout);
                break;
            }

            if (c != line[cindex]) {
                fprintf(stdout, "Received wrong char '%c', code %d = 0%03o = 0x%02x expected ''%c'\n", c, c, c, c, line[cindex]);
            }
            // else {
            //     fprintf(stdout, "Received code %d = 0%03o = 0x%02x\n", c, c, c);
            // }
            fflush(stdout);

            cindex++;

            if (c == 3) {
                fclose(fp);
                if (line) {
                    free(line);
                }
                if (term_restore(&term)) {
                    fprintf(stderr, "Cannot restore standard input state: %s.\n", strerror(errno));
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            }
        }
    }

    fclose(fp);
    if (line) {
        free(line);
    }
    if (term_restore(&term)) {
        fprintf(stderr, "Cannot restore standard input state: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
