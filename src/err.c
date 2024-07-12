#include "err.h"


/*
 * Using error codes from ErrorCode,
 * defined in err.h header file.
 * (Possibly switch to switch statement)
 */
const char *ERR_MESSAGES[] = {
    "No error",                                                                 /* ERR_NONE                         */
    "Pointer to NULL error",                                                    /* ERR_POINTER_TO_NULL              */
    "I/O error",                                                                /* ERR_IO_ERROR                     */
    "Error reading source",                                                     /* ERR_GENERIC_FILER_ERROR          */
    "Error using temporary file for local cache",                               /* ERR_TMP_FILE_READ_ERRPR          */
    "Error setting up curl, to pull source",                                    /* ERR_CURL_SETUP_ERROR             */
    "Error reading from specified file",                                        /* ERR_FILE_READ_ERROR              */
    "Error reading from specified URL",                                         /* ERR_URL_READ_ERROR               */
    "Error terminal stream (stdin) or its descriptor is invalid",               /* ERR_TERM_STREAM_ERROR            */
    "Error setting up terminal",                                                /* ERR_TERM_SETUP_ERROR             */
    "Error backing up terminal settings",                                       /* ERR_TERM_BACKUP_ERROR            */
    "Error setting/restoring necessary terminal settings",                      /* ERR_TERM_ATTRIBUTE_SETTING_ERROR */
    "Error encountered in typer",                                               /* ERR_GENERIC_TYPER_ERROR          */
    "Error, missing required argument\nRun with --help for more information",   /* ERR_MISSING_ARGUMENT             */
    "Invalid argument\nRun with --help for more information",                   /* ERR_INVALID_ARGUMENT             */
    "Error, incompatible arguments\nRun with --help for more information",      /* ERR_INCOMPATIBLE_ARGUMENTS       */
};

const char* stt_err_str(const ErrorCode err) {
    if (sizeof(ERR_MESSAGES)/sizeof(char*) > err){
        return ERR_MESSAGES[err];
    }
    return "";
}
