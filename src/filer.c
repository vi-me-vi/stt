#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>

#include "err.h"
#include "filer.h"



/* Callback for curl writing */
static size_t write_data(const void *ptr, const size_t size, const size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}


/*
 * Helper function to read text from web to tmp file
 * More info: https://curl.se/libcurl/c/url2file.html
 * TODO: add error handling for curl
*/
static ErrorCode read_from_web(FILE **fp, const char *url) {
    CURL *curl_handle;

    *fp = tmpfile();  /* Consider using fmemopen (already using POSIX functions in typer module) */
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    int c_res_err = 0;
    c_res_err += curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    c_res_err += curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    c_res_err += curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    c_res_err += curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    if (c_res_err != 0) {
        return ERR_CURL_SETUP_ERROR;
    }

    if(fp) {
        if (curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, *fp) != CURLE_OK) {
            return ERR_CURL_SETUP_ERROR;
        }
        if (curl_easy_perform(curl_handle)) {
            return ERR_URL_READ_ERROR;
        }
    } else {
        return ERR_TMP_FILE_READ_ERRPR;
    }

    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    rewind(*fp);

    return ERR_NONE;
}


/* Helper function to read from local file */
static ErrorCode read_from_file(FILE **fp, const char* location) {
    *fp = fopen(location, "r");
    if (!*fp) {
        return ERR_FILE_READ_ERROR;
    }
    return ERR_NONE;
}


/*
 * Function to get correct file for typing input
 * Read modes: 'w' - web, 'l' - local file
*/
ErrorCode run_filer(FILE **fp, const char *read_location, const char read_mode) {
    if (read_mode == 'w') {
        return read_from_web(fp, read_location);
    } else if (read_mode == 'l') {
        return read_from_file(fp, read_location);
    }
    return ERR_GENERIC_FILER_ERROR;
}
