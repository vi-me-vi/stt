#ifndef STT_ARGS_H
#define STT_ARGS_H

#include <stdbool.h>

#include "err.h"



/* Hold all configs in one place */
typedef struct stt_conf {
    char filer_mode;
    char *source;
    bool preserve_formatting;
    bool monochrome_mode;
} SttConf;

ErrorCode stt_parse_args(const int, char*[], SttConf*);

#endif
