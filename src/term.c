#define  _GNU_SOURCE
// #define _POSIX_SOURCE 1
// #define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdlib.h>
#include <term.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>

#include "err.h"


/* Inspired by: https://cboard.cprogramming.com/c-programming/157438-capturing-keyboard-input-one-character-time.html#post1165318 */

/*
 * Helper function change the terminal related to the stream to "raw" state.
 * (Usually you call this with stdin).
 * This means you get all keystrokes, and special keypresses like CTRL-C
 * no longer generate interrupts.
 *
 * You must restore the state before your program exits, or your user will
 * frantically have to figure out how to type 'reset' blind, to get their terminal
 * back to a sane state.
*/
ErrorCode term_init(const SttTerm *term) {
    struct termios old, raw, actual;
    int fd;

    if (!term->stream) {
        return ERR_TERM_STREAM_ERROR;
    }

    /* Tell the C library not to buffer any data from/to the stream. */
    if (setvbuf(term->stream, NULL, _IONBF, 0)) {
        return ERR_TERM_SETUP_ERROR;
    }

    /* Write/discard already buffered data in the stream. */
    fflush(term->stream);

    fd = fileno(term->stream);
    if (fd == -1) {
        return ERR_TERM_STREAM_ERROR;
    }

    /* Get current terminal settings. */
    if (tcgetattr(fd, &old)) {
        return ERR_TERM_BACKUP_ERROR;
    }

    if (term->save_state) {
        *term->save_state = old;
    }

    /* New terminal settings are based on current ones. */
    raw = old;

    /* Because the terminal needs to be restored to the original state,
     * you want to ignore CTRL-C (break). */
    raw.c_iflag |= IGNBRK;  /* do ignore break, */
    raw.c_iflag &= ~BRKINT; /* do not generate INT signal at break. */

    /* Allow to eceive data. */
    raw.c_cflag |= CREAD;

    /* Do not generate signals from special keypresses. */
    raw.c_lflag &= ~ISIG;

    /* Do not echo characters. */
    raw.c_lflag &= ~ECHO;

    /* Most importantly, disable "canonical" mode. */
    raw.c_lflag &= ~ICANON;

    raw.c_cc[VMIN] = 1;  /* Wait until at least one character available, */
    raw.c_cc[VTIME] = 0; /* Wait indefinitely. */

    /* Set the new terminal settings. */
    if (tcsetattr(fd, TCSAFLUSH, &raw)) {
        return ERR_TERM_ATTRIBUTE_SETTING_ERROR;
    }

    /* Verify if ALL necessary settings were applied */
    if (tcgetattr(fd, &actual)) {
        /* Try to restore old settings */
        tcsetattr(fd, TCSANOW, term->save_state);
        return ERR_TERM_ATTRIBUTE_SETTING_ERROR;
    }

    if (
        actual.c_iflag != raw.c_iflag ||
        actual.c_oflag != raw.c_oflag ||
        actual.c_cflag != raw.c_cflag ||
        actual.c_lflag != raw.c_lflag
    ){
        /* Try to restore the old settings */
        tcsetattr(fd, TCSANOW, term->save_state);
        return ERR_TERM_ATTRIBUTE_SETTING_ERROR;
    }

    return ERR_NONE;
}

/* Helper function to restore the saved state */
ErrorCode term_restore(const SttTerm *term) {
    int fd, result;

    if (!term->stream || !term->save_state) {
        return ERR_TERM_STREAM_ERROR;
    }

    /* Write/discard all buffered data in the stream. Ignores errors. */
    fflush(term->stream);

    fd = fileno(term->stream);
    if (fd == -1) {
        return ERR_TERM_SETUP_ERROR;
    }

    /* Restore terminal state. */
    do {
        result = tcsetattr(fd, TCSAFLUSH, term->save_state);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
        return ERR_TERM_ATTRIBUTE_SETTING_ERROR;
    }

    return ERR_NONE;
}

