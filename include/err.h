#ifndef STT_ERR_H
#define STT_ERR_H

/* NOTE: If changed, please update error messages located in err.c */
typedef enum error_code {
    ERR_NONE = 0,
    ERR_POINTER_TO_NULL,
    ERR_IO_ERROR,
    ERR_GENERIC_FILER_ERROR,
    ERR_TMP_FILE_READ_ERRPR,
    ERR_CURL_SETUP_ERROR,
    ERR_FILE_READ_ERROR,
    ERR_URL_READ_ERROR,
    ERR_TERM_STREAM_ERROR,
    ERR_TERM_SETUP_ERROR,
    ERR_TERM_BACKUP_ERROR,
    ERR_TERM_ATTRIBUTE_SETTING_ERROR,
    ERR_GENERIC_TYPER_ERROR,
    ERR_MISSING_ARGUMENT,
    ERR_INVALID_ARGUMENT,
    ERR_INCOMPATIBLE_ARGUMENTS,
} ErrorCode;


const char* stt_err_str(const ErrorCode);

#endif
