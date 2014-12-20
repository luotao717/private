#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


int http_get(const char *server, char *buf, size_t count, off_t offset);
int http_post(const char *server, char *buf, size_t count);
int http_upload(const char *server, char *file, const char *content_name);
int http_download(const char * url, const char *fileName);
