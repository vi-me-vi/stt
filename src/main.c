#include <curl/curl.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "err.h"
#include "filer.h"
#include "term.h"
#include "typer.h"



const char* LOGO  = " ______     ______   ______  \n\
                   \r/\\  ___\\   /\\__  _\\ /\\__  _\\ \n\
                   \r\\ \\___  \\  \\/_/\\ \\/ \\/_/\\ \\/ \n\
                   \r \\/\\_____\\    \\ \\_\\    \\ \\_\\ \n\
                   \r  \\/_____/     \\/_/     \\/_/ \n\
                   \r   Simple    Terminal   Typer\n\n";

const char* HELPTEXT = "Usage: stt [OPTION] SOURCE\n \
                      \r\n \
                      \rOptions:\n \
                      \r  -h,--help\t\t\tDisplay help message\n \
                      \r  -f,--file\t\t\t(REQUIRED, excludes -u) Falg used to set \"local mode\", to use local file as source\n \
                      \r  -u,--url \t\t\t(REQUIRED, excludes -f) Flag used to set \"web mode\", to use URL to a webpage as source\n \
                      \r  -p,--preserve-formatting\tPreserve original formatting of the source\n \
                      \r  -m,--monochrome\t\tMonochrome mode (\x1B[07merror\x1B[00m | \x1B[04mcorrect input\x1B[00m)\n";

const int MIN_ARGS  = 2;
const int MAX_ARGS = 4;


/* Hold all configs in one place */
typedef struct Conf {
    char filer_mode;
    char* source;
    bool preserve_formatting;
    bool monochrome_mode;
} Conf;


ErrorCode handle_args(const char, const char*, Conf*);


int main(int argc, char *argv[]) {
    ErrorCode err     = ERR_NONE;
    FILE *fp          = NULL;
    Conf config       = {
        .filer_mode          = '\0',
        .source              = NULL,
        .preserve_formatting = false,
    };

    /* Print logo */
    printf("%s", LOGO);

    /* Check usage */
    if (argc < MIN_ARGS || argc > MAX_ARGS) {
        printf("Usage: %s [OPTIONS] SOURCE", argv[0]);
        return EXIT_SUCCESS;
    }

    /* Process args */
    for (int i = 1; i < argc; i++) {
        if (strlen(argv[i]) > 1 && argv[i][0] == '-' && argv[i][1] == '-') {  /* Handle --arg type arguments */
            if (err = handle_args(argv[i][2], argv[i], &config), err != ERR_NONE) {
                goto handle_error;
            }
        } else if (strlen(argv[i]) > 1 && argv[i][0] == '-') {  /* Handle -h and -pf type arguments */
            for (size_t arg_i = 1; arg_i < strlen(argv[i]); arg_i++) {
                if (err = handle_args(argv[i][arg_i], NULL, &config), err != ERR_NONE) {
                    goto handle_error;
                }
            }
        } else if (config.source == NULL) {  /* Handle setting source in any placement */
            config.source = argv[i];
        } else {
            err = ERR_INVALID_ARGUMENT;
            goto handle_error;
        }
    }

    /* Check parsed config and proceed */
    if (config.filer_mode == '\0') {
        err = ERR_MISSING_ARGUMENT;
        goto handle_error;
    }

    if (config.source == NULL) {
        err = ERR_MISSING_ARGUMENT;
        goto handle_error;
    }

    if (err = run_filer(&fp, config.source, config.filer_mode), err != ERR_NONE) {
        goto handle_error;
    }

    /* Set-up terminal */
    SttTerm term = {
        .stream = stdin,
        .save_state = (struct termios *) malloc(sizeof(struct termios))
    };

    if (err = term_init(&term), err != ERR_NONE) {
        goto handle_cleanup_and_error;
    }

    /* Typer */
    if (run_typer(fp, config.preserve_formatting, config.monochrome_mode) != ERR_NONE) {
        err = ERR_GENERIC_TYPER_ERROR;
        ErrorCode secondary_err = ERR_NONE;
        if (secondary_err = term_restore(&term), secondary_err != ERR_NONE) {
            fprintf(stderr, "Secondary ERROR: %s\n", stt_err_str(secondary_err));
        }
        goto handle_cleanup_and_error;
    }

    /* Clean up & restore terminal */
    if (err = term_restore(&term), err != ERR_NONE) {
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
    if (err != ERR_NONE) {
        fprintf(stderr, "%s\n", stt_err_str(err));
    }
    fflush(stderr);
    return EXIT_FAILURE;
}


ErrorCode handle_args(const char arg, const char *full_arg, Conf *config) {
    switch (arg) {
        /* Display help and exit */
        case 'h':
            if (full_arg != NULL && strcmp(full_arg, "--help") != 0) {
                return ERR_INVALID_ARGUMENT;
            }
            printf("%s", HELPTEXT);
            exit(EXIT_SUCCESS);
        /* Set source mode to local file */
        case 'f':
            if (full_arg != NULL && strcmp(full_arg, "--file") != 0) {
                return ERR_INVALID_ARGUMENT;
            }
            if (config->filer_mode != '\0') {
                return ERR_INCOMPATIBLE_ARGUMENTS;
            }
            config->filer_mode = 'l';
            break;
        /* Set source mode to URL */
        case 'u':
            if (full_arg != NULL && strcmp(full_arg, "--url") != 0) {
                return ERR_INVALID_ARGUMENT;
            }
            if (config->filer_mode != '\0') {
                return ERR_INCOMPATIBLE_ARGUMENTS;
            }
            config->filer_mode = 'w';
            break;
        /* Set config to preserve original formatting */
        case 'p':
            if (full_arg != NULL && strcmp(full_arg, "--preserve-formatting") != 0) {
                return ERR_INVALID_ARGUMENT;
            }
            config->preserve_formatting = true;
            break;
        /* Set config to use moinochrome hilighting */
        case 'm':
            if (full_arg != NULL && strcmp(full_arg, "--monochrome") != 0) {
                return ERR_INVALID_ARGUMENT;
            }
            config->monochrome_mode = true;
            break;
        /* Handle invalid options */
        default:
            return ERR_INVALID_ARGUMENT;
    }
    return ERR_NONE;
}
