#include <curl/curl.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
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
#define HELPTEXT "Usage: stt [OPTION] SOURCE\n \
                \r\n \
                \rOptions:\n \
                \r  -h,--help\t\t\tDisplay help message\n \
                \r  -f,--file\t\t\t(REQUIRED, excludes -u) Falg used to set \"local mode\", to use local file as source\n \
                \r  -u,--url \t\t\t(REQUIRED, excludes -f) Flag used to set \"web mode\", to use URL to a webpage as source\n \
                \r  -p,--preserve-formatting\tPreserve original formatting of the source\n"



/* Hold all configs in one place */
typedef struct Conf {
    char filer_mode;
    char* source;
    bool preserve_formatting;
} Conf;


int handle_args(const char, const char*, Conf*, char**);


int main(int argc, char *argv[]) {
    char *err_message = NULL;
    FILE *fp          = NULL;
    Conf config       = {
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
            if (handle_args(argv[i][2], argv[i], &config, &err_message) != 0) {
                goto handle_error;
            }
        } else if (strlen(argv[i]) > 1 && argv[i][0] == '-') {  /* Handle -h and -pf type arguments */
            for (size_t arg_i = 1; arg_i < strlen(argv[i]); arg_i++) {
                if (handle_args(argv[i][arg_i], NULL, &config, &err_message) != 0) {
                    goto handle_error;
                }
            }
        } else if (config.source == NULL) {  /* Handle setting source in any placement */
            config.source = argv[i];
        } else {
            err_message = "Multiple sources provided";
            goto handle_error;
        }
    }

    /* Check parsed config and proceed */
    if (config.filer_mode == '\0') {
        err_message = "Source mode not defined";
        goto handle_error;
    }

    if (config.source == NULL) {
        err_message = "Source not defined";
        goto handle_error;
    }

    if (run_filer(&fp, config.source, config.filer_mode) != 0) {
        err_message = "Error reading from source";
        goto handle_error;
    }

    /* Set-up terminal */
    SttTerm term = {
        .stream = stdin,
        .save_state = (struct termios *) malloc(sizeof(struct termios))
    };

    if (term_init(&term) != 0) {
        err_message = "Cannot make standard input raw";
        goto handle_cleanup_and_error;
    }

    /* Typer */
    if (run_typer(fp, config.preserve_formatting) != 0) {
        err_message = "Typer error";  /* TODO: better error message */
        if (term_restore(&term) != 0) {
            fprintf(stderr, "Warning: Cannot restore standard input\n");
        }
        goto handle_cleanup_and_error;
    }

    /* Clean up & restore terminal */
    if (term_restore(&term) != 0) {
        err_message = "Cannot restore standard input";
        goto handle_cleanup_and_error;
    }

    fclose(fp);
    free(term.save_state);
    return EXIT_SUCCESS;

/* Error handling block */
handle_cleanup_and_error:
    fclose(fp);
    free(term.save_state);
handle_error:
    if (err_message != NULL) {
        fprintf(stderr, "%s\n", err_message);
    }
    fflush(stderr);
    return EXIT_FAILURE;
}


int handle_args(const char arg, const char *full_arg, Conf *config, char **err_msg) {
    switch (arg) {
        /* Display help and exit */
        case 'h':
            if (full_arg != NULL && strcmp(full_arg, "--help") != 0) {
                *err_msg = "Unknown argument\nRun with --help for more information";
                return 1;
            }
            printf(HELPTEXT);
            exit(EXIT_SUCCESS);
        /* Set source mode to local file */
        case 'f':
            if (full_arg != NULL && strcmp(full_arg, "--file") != 0) {
                *err_msg = "Unknown argument\nRun with --help for more information";
                return 1;
            }
            if (config->filer_mode != '\0') {
                *err_msg = "Incompatible/multiple source modes";
                return 1;
            }
            config->filer_mode = 'l';
            break;
        /* Set source mode to URL */
        case 'u':
            if (full_arg != NULL && strcmp(full_arg, "--url") != 0) {
                *err_msg = "Unknown argument\nRun with --help for more information";
                return 1;
            }
            if (config->filer_mode != '\0') {
                *err_msg = "Incompatible/multiple source modes";
                return 1;
            }
            config->filer_mode = 'w';
            break;
        /* Set config to preserve original formatting */
        case 'p':
            if (full_arg != NULL && strcmp(full_arg, "--preserve-formatting") != 0) {
                *err_msg = "Unknown argument\nRun with --help for more information";
                return 1;
            }
            config->preserve_formatting = true;
            break;
        /* Handle invalid options */
        default:
            *err_msg = "Unknown argument\nRun with --help for more information";
            return 1;
    }
    return 0;
}
