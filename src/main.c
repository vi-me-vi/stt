#include <curl/curl.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "filer.h"
#include "term.h"
#include "typer.h"



#define LOGO " ______     ______   ______  \n\
            \r/\\  ___\\   /\\__  _\\ /\\__  _\\ \n\
            \r\\ \\___  \\  \\/_/\\ \\/ \\/_/\\ \\/ \n\
            \r \\/\\_____\\    \\ \\_\\    \\ \\_\\ \n\
            \r  \\/_____/     \\/_/     \\/_/ \n\
            \r   Simple    Terminal   Typer\n\n"
#define MIN_ARGS 2
#define MAX_ARGS 4
#define HELPTEXT "Usage:·stt·[OPTION]·SOURCE\n \
                \r\n \
                \rOptions:\n \
                \r  -h,--help\t\t\tDisplay help message\n \
                \r  -f,--file\t\t\tUse local file as source\n \
                \r  -u,--url \t\t\tUse URL to a webpage as source\n \
                \r  -p,--preserve_formatting\tPreserve original formatting of the source\n"



typedef struct conf {
    char filer_mode;
    char* source;
    bool preserve_formatting;
} conf;


int handle_args(char, conf*);


int main(int argc, char *argv[]) {
    FILE *fp      = NULL;
    int opt_count = 0;
    conf config   = {
        .filer_mode          = '\0',
        .source              = NULL,
        .preserve_formatting = false,
    };

    /* Print logo */
    printf(LOGO);

    /* Check usage */
    if (argc < MIN_ARGS || argc > MAX_ARGS) {
        printf("Usage: %s [OPTIONS] SOURCE", argv[0]);
        return EXIT_SUCCESS;
    }

    /* Process args */
    for (int i = 1; i < argc; i++) {
        if (strlen(argv[i]) > 1 && argv[i][0] == '-' && argv[i][1] == '-') {  /* Handle --arg type arguments */
            handle_args(argv[i][2], &config);
            opt_count++;
        } else if (strlen(argv[i]) > 1 && argv[i][0] == '-') {  /* Handle -h and -pf type arguments */
            for (size_t arg_i = 1; arg_i < strlen(argv[i]); arg_i++) {
                handle_args(argv[i][arg_i], &config);
            }
            opt_count++;
        } else if (i < argc - 1) {  /* Ignore last arg, ince it defines source */
            printf("Unknown argument\nRun with --help for more information\n");
            return EXIT_FAILURE;
        }
    }

    /* Check parsed config and proceed */
    if (config.filer_mode == '\0') {
        fprintf(stderr, "Source mode not defined\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    if (opt_count == argc - 1) {
        fprintf(stderr, "Source not defined");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    config.source = argv[argc-1];
    if (run_filer(&fp, config.source, config.filer_mode)) {
        fprintf(stderr, "Error reading from source\n");
        fflush(stderr);
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
    run_typer(fp, config.preserve_formatting);

    /* Clean up & restore terminal */
    fclose(fp);
    if (term_restore(&term)) {
        fprintf(stderr, "Cannot restore standard input state: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


int handle_args(char arg, conf *config) {
    switch (arg) {
        case 'h':
            printf(HELPTEXT);
            exit(EXIT_SUCCESS);
        case 'f':
            if (config->filer_mode != '\0') {
                fprintf(stderr, "Incompatible/multiple source modes\n");
                fflush(stderr);
                return EXIT_FAILURE;
            }
            config->filer_mode = 'l';
            break;
        case 'u':
            if (config->filer_mode != '\0') {
                fprintf(stderr, "Incompatible/multiple source modes\n");
                fflush(stderr);
                return EXIT_FAILURE;
            }
            config->filer_mode = 'w';
            break;
        case 'p':
            config->preserve_formatting = true;
            break;
        default:
            printf("Unknown argument\nRun with --help for more information\n");
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
