#ifndef STT_TERM_H
#define STT_TERM_H


#include <stdio.h>


typedef struct SttTerm {
    FILE *const stream;
    struct termios *const save_state;
} SttTerm;

int term_init(const SttTerm*);
int term_restore(const SttTerm*);

#endif
