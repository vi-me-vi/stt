#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "err.h"


const char *LOGO  = " ______     ______   ______  \n\
                   \r/\\  ___\\   /\\__  _\\ /\\__  _\\ \n\
                   \r\\ \\___  \\  \\/_/\\ \\/ \\/_/\\ \\/ \n\
                   \r \\/\\_____\\    \\ \\_\\    \\ \\_\\ \n\
                   \r  \\/_____/     \\/_/     \\/_/ \n\
                   \r   Simple    Terminal   Typer\n\n";

const char *HELPTEXT = "Usage: stt [OPTION] SOURCE\n \
                      \r\n \
                      \rOptions:\n \
                      \r  -h,--help\t\t\tDisplay help message\n \
                      \r  -f,--file\t\t\t(REQUIRED, excludes -u) Falg used to set \"local mode\", to use local file as source\n \
                      \r  -u,--url \t\t\t(REQUIRED, excludes -f) Flag used to set \"web mode\", to use URL to a webpage as source\n \
                      \r  -p,--preserve-formatting\tPreserve original formatting of the source\n \
                      \r  -m,--monochrome\t\tMonochrome mode (\x1B[07merror\x1B[00m | \x1B[04mcorrect input\x1B[00m)\n";

const int MIN_ARGS = 2;
const int MAX_ARGS = 5;


ErrorCode handle_args(const char arg, const char *full_arg, SttConf *config) {
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


ErrorCode stt_parse_args(const int argc, char * argv[], SttConf *conf) {
    ErrorCode err = ERR_NONE;

    /* Print logo */
    printf("%s", LOGO);

    /* Check usage */
    if (argc < MIN_ARGS || argc > MAX_ARGS) {
        printf("Usage: %s [OPTIONS] SOURCE", argv[0]);
        exit(EXIT_SUCCESS);
    }

    /* Process args */
    for (int i = 1; i < argc; i++) {
        if (strlen(argv[i]) > 1 && argv[i][0] == '-' && argv[i][1] == '-') {  /* Handle --arg type arguments */
            if (err = handle_args(argv[i][2], argv[i], conf), err != ERR_NONE) {
                return err;
            }
        } else if (strlen(argv[i]) > 1 && argv[i][0] == '-') {  /* Handle -h and -pf type arguments */
            for (size_t arg_i = 1; arg_i < strlen(argv[i]); arg_i++) {
                if (err = handle_args(argv[i][arg_i], NULL, conf), err != ERR_NONE) {
                    return err;
                }
            }
        } else if (conf->source == NULL) {  /* Handle setting source in any placement */
            conf->source = argv[i];
        } else {
            return ERR_INVALID_ARGUMENT;
        }
    }
    return ERR_NONE;
}


