#include <curl/curl.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "filer.h"
#include "term.h"
#include "typer.h"



#define MIN_ARGS 2
#define MAX_ARGS 3



int main(int argc, char *argv[]) {
    FILE *fp;

    /* Print logo */
    printf(" ______     ______   ______  \n\
          \r/\\  ___\\   /\\__  _\\ /\\__  _\\ \n\
          \r\\ \\___  \\  \\/_/\\ \\/ \\/_/\\ \\/ \n\
          \r \\/\\_____\\    \\ \\_\\    \\ \\_\\ \n\
          \r  \\/_____/     \\/_/     \\/_/ \n\
          \r   Simple    Terminal   Typer\n\n");

    /*
     * ARGS:
     * -h,--help
     *  -f,--file
     *  -u,--url
    */
    if (argc < MIN_ARGS || argc > MAX_ARGS) {
        printf("Usage: %s [OPTION] SOURCE", argv[0]);
        return EXIT_SUCCESS;
    }

    /* Process args */
    if (strcmp("--help", argv[1]) == 0 || strcmp("-h", argv[1]) == 0) {
        printf("Usage:·%s·[OPTION]·SOURCE\n \
                \r\n \
                \rOptions:\n \
                \r  -h,--help\tDisplay help message\n \
                \r  -f,--file\tTEXT Path to file, used for typing\n \
                \r  -u,--url \tTEXT URL of a PLAINTEXT page, used for typing\n",
            argv[0]
        );
        return EXIT_SUCCESS;
    } else if (strcmp(argv[1], "--file") == 0 || strcmp(argv[1], "-f") == 0) {
        if (argc < 3) {  /* Handle no actual filepath passed */
            printf("Usage: %s %s SOURCE", argv[0], argv[1]);
            return EXIT_SUCCESS;
        }
        if (run_filer(&fp, argv[2], 'l')) {
            fprintf(stderr, "Error opening file\n");
            fflush(stderr);
            return EXIT_FAILURE;
        }
    } else if (strcmp(argv[1], "--url") == 0 || strcmp(argv[1], "-u") == 0) {
        if (argc < 3) {  /* Handle no actual URL passed */
            printf("Usage: %s %s SOURCE", argv[0], argv[1]);
            return EXIT_SUCCESS;
        }
        if (run_filer(&fp, argv[2], 'w')) {
            fprintf(stderr, "Error pulling text\n");
            fflush(stderr);
            return EXIT_FAILURE;
        }
    } else {
        printf("Unknown option\nRun with --help for more information");
        return EXIT_FAILURE;
    }

    /* Set-up terminal */
    SttTerm term = {
        .stream = stdin,
        .save_state = (struct termios *) malloc(sizeof(struct termios))
    };

    if (term_init(&term)) {
        fprintf(stderr, "Cannot make standard input raw: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    /* Typer */
    run_typer(fp);

    /* Clean up & restore terminal */
    fclose(fp);
    if (term_restore(&term)) {
        fprintf(stderr, "Cannot restore standard input state: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
