#include <curl/curl.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "args.h"
#include "err.h"
#include "filer.h"
#include "term.h"
#include "typer.h"



int main(int argc, char *argv[]) {
    ErrorCode err     = ERR_NONE;
    FILE *fp          = NULL;
    SttConf config       = {
        .filer_mode          = '\0',
        .source              = NULL,
        .preserve_formatting = false,
    };

    /*
     * Parse CLI args into SttConfig structure
     * NOTE: exits (without returning) on empty args and on help
     */
    if (err = stt_parse_args(argc, argv, &config), err != ERR_NONE) {
        goto handle_error;
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
