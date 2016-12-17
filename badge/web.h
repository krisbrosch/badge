#ifndef WEB_H
#define WEB_H

#include "http.h"

struct web_DynamicPath {
  char *pathName;
  void (*handle)(struct HttpRequest *);
};

struct web_StaticPath {
  char *pathName;
  char *content;
};

extern const struct web_StaticPath web_staticPaths[];
extern const struct web_DynamicPath web_dynamicPaths[];

#endif
