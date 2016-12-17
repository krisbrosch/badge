#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "web.h"
#include "http.h"
#include "leddemos.h"

void wp_test(struct HttpRequest *request) {
  char buf[200];
  char *nstr;
  int n;
  nstr = http_getUrlParam(request, "n");
  n = atoi(nstr);
  snprintf(buf, sizeof(buf), "Your number was: %d = 0x%08x\n", n, n);
  buf[199] = 0;
  http_reply_ok(request, (uint8_t *)buf, strlen(buf));
}

const struct web_StaticPath web_staticPaths[] = {
  {"/", "<html><body bgcolor=\"green\"><h1>HELLO HTTP!</h1></body></html>\r\n"},
  { NULL, NULL }
};

const struct web_DynamicPath web_dynamicPaths[] = {
  { "/test", &wp_test },
  { "/demo", &wp_demo },
  { NULL, NULL }
};



