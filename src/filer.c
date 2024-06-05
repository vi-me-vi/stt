#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
static int read_from_web(FILE **fp, const char *url) {
    CURL *curl_handle;

    *fp = tmpfile();  /* Consider using fmemopen (already using POSIX functions in typer module) */
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    if(fp) {
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, *fp);
        if (curl_easy_perform(curl_handle)) {
            return EXIT_FAILURE;
        }
    }

    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();


    rewind(*fp);
    return EXIT_SUCCESS;
}


/* Helper function to read from local file */
static int read_from_file(FILE **fp, const char* location) {
    *fp = fopen(location, "r");
    if (!*fp) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


/*
 * Function to get correct file for typing input
 * Read modes: 'w' - web, 'l' - local file
*/
int run_filer(FILE **fp, const char *read_location, const char read_mode) {
    if (read_mode == 'w') {
        return read_from_web(fp, read_location);
    } else if (read_mode == 'l') {
        return read_from_file(fp, read_location);
    }
    return EXIT_FAILURE;
}
