#ifndef HTTP_H
#define HTTP_H

#include <stdint.h>

#define MAX_HTTP_LINE 200
#define MAX_URL_PARAMS 8

struct HttpRequest {
  char pathqs[MAX_HTTP_LINE];
  char *path;
  char *qs;
  char *urlparams[MAX_URL_PARAMS][2];
  int nurlparams;
  int channel;
};

uint8_t hexToNibble(char c);
char *http_getUrlParam(struct HttpRequest *req, char *paramName);
void http_reply_ok(struct HttpRequest *request, uint8_t *response, uint32_t length);
void http_loop();

#endif
