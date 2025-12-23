#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"


struct Memory {
    char *buf;
    size_t len;
};

static size_t write_cb(void *data, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    struct Memory *mem = userdata;

    char *newbuf = realloc(mem->buf, mem->len + total + 1);
    if (!newbuf) return 0; // возврат 0 скажет libcurl об ошибке

    mem->buf = newbuf;
    memcpy(&(mem->buf[mem->len]), data, total);
    mem->len += total;
    mem->buf[mem->len] = 0;
    return total;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s city_name\n", argv[0]);
        return 1;
    }

    char url[256];

    snprintf(url, sizeof(url), "https://wttr.in/%s?format=j1", argv[1]);
    printf("%s\n", url);
    CURL *curl;
    CURLcode res;

    struct Memory chunk;
    chunk.buf = malloc(1);  // Start with 1 byte (will be resized)
    chunk.len = 0;              // No data at first

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "curl init failed\n");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    res = curl_easy_perform(curl);
    printf("%s\n", chunk.buf);
    // Check for errors
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    // Clean up
    curl_easy_cleanup(curl);
    free(chunk.buf);
    curl_global_cleanup();
    return 0;
}
