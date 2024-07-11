#ifndef STT_TERM_H
#define STT_TERM_H


#include <stdio.h>

#include "err.h"


typedef struct stt_term {
    FILE *const stream;
    struct termios *const save_state;
} SttTerm;

ErrorCode term_init(const SttTerm*);
ErrorCode term_restore(const SttTerm*);

#endif
