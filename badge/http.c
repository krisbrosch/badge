#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "http.h"
#include "driverlib.h"
#include "wifi.h"
#include "web.h"


char http_buf[500];


// Converts ascii-hex character to 4-bit value
// Returns 0xff for invalid characters
uint8_t hexToNibble(char c) {
  if((c >= '0') && (c <= '9')) {
    return (uint8_t)(c - '0');
  } else if((c >= 'A') && (c <= 'F')) {
    return (uint8_t)(c - 'A') + 0xa;
  } else if((c >= 'a') && (c <= 'f')) {
    return (uint8_t)(c - 'a') + 0xa;
  }
  return 0xff;
}

// In-place URL-decode of a null terminated string
// Yes, %00 will terminate the result
void urldecode(char *s) {
  char *in = s;
  char *out = s;
  char a, b;
  uint8_t x;
  while(*in != 0) {
    if(*in == '+') {
      *out = ' ';
    } else if(*in == '%') {
      in++;
      a = *in++;
      if(a == 0) {
        break;
      } else {
        x = hexToNibble(a) << 4;
      }
      b = *in;
      if(b == 0) {
        break;
      } else {
        x |= hexToNibble(b);
      }
      *out = (char)x;
    } else {
      *out = *in;
    }
    in++;
    out++;
  }
  *out = 0;
}

// Parse the first line of an HTTP request. The query string will be
// in-place split up into urlparams, and all parameters and the path will
// be in-place url-decoded.
void http_parseRequest(struct HttpRequest *req, char *request, int channel) {
  char *s, *p, *end, *sav;

  req->channel = channel;

  s = request;
  end = s + strlen(s);

  // method
  while((*s != ' ') && (s < end)) {
    s++;
  }
  s++;

  // path
  p = req->pathqs;
  req->path = p;
  while((*s != '?') && (*s != ' ') && (s < end)) {
    if(p - req->pathqs < MAX_HTTP_LINE-1)
      *p++ = *s++;
    else
      s++;
  }
  *p++ = 0;

  // query string
  req->qs = p;
  if(*s == '?') {
    s++;
    while((*s != ' ') && (s < end)) {
      if(p - req->pathqs < MAX_HTTP_LINE-1)
        *p++ = *s++;
      else
        s++;
    }
  }
  *p = 0;

  // decode path
  urldecode(req->path);

  // parse query string
  req->nurlparams = 0;
  s = strtok_r(req->qs, "&", &sav);
  while(s) {
    p = s;
    while((*p != '=') && (*p != 0)) {
      p++;
    }
    if(*p == '=') {
      *p++ = 0;
    }
    if(req->nurlparams < MAX_URL_PARAMS) {
      urldecode(s);
      urldecode(p);
      req->urlparams[req->nurlparams][0] = s;
      req->urlparams[req->nurlparams][1] = p;
      req->nurlparams++;
    }
    s = strtok_r(0, "&", &sav);
  }
}

// Get the value of a url parameter with given parameter name
// Parameter names are case-sensitive
// Returns a pointer into the parsed and url-decoded query string, or NULL
char *http_getUrlParam(struct HttpRequest *req, char *paramName) {
  int i;
  for(i = 0; i < req->nurlparams; i++) {
    if(strcmp(req->urlparams[i][0], paramName) == 0) {
      return req->urlparams[i][1];
    }
  }
  return 0;
}

void http_reply_404(struct HttpRequest *request) {
  char *response = "HTTP/1.0 404 NOT FOUND\r\n\r\n:(";
  wifi_send(request->channel, (uint8_t *)response, strlen(response));
  wifi_close(request->channel);
}

void http_reply_ok(struct HttpRequest *request, uint8_t *response, uint32_t length) {
  char *headers = "HTTP/1.0 200 OK\r\n\r\n";
  wifi_send(request->channel, (uint8_t *)headers, strlen(headers));
  wifi_send(request->channel, response, length);
  wifi_close(request->channel);
}

void http_handle(int channel, char *buf) {
  struct HttpRequest request;
  int i;

  http_parseRequest(&request, http_buf, channel);
  for(i = 0; web_staticPaths[i].pathName != NULL; i++) {
    if(strcmp(web_staticPaths[i].pathName, request.path) == 0) {
      http_reply_ok(&request, (uint8_t *)web_staticPaths[i].content, strlen(web_staticPaths[i].content));
      return;
    }
  }

  for(i = 0; web_dynamicPaths[i].pathName != NULL; i++) {
    if(strcmp(web_dynamicPaths[i].pathName, request.path) == 0) {
      web_dynamicPaths[i].handle(&request);
      return;
    }
  }

  http_reply_404(&request);
}

void http_loop() {
  int channel;
  uint32_t length;

  channel = wifi_poll(&length, (uint8_t *)http_buf, sizeof(http_buf));
  if(channel == -1) {
    return;
  }

  if(length > sizeof(http_buf))
    length = sizeof(http_buf);
  http_buf[length-1] = 0;

  http_handle(channel, http_buf);
}


