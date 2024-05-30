// #define  _GNU_SOURCE
#define _POSIX_SOURCE 1
#define  _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "term.h"
#include "typer.h"



int main(void) {
    FILE *fp;

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

    run_typer(fp);

    fclose(fp);
    if (term_restore(&term)) {
        fprintf(stderr, "Cannot restore standard input state: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
