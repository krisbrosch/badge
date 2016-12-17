#ifndef LEDDEMOS_H
#define LEDDEMOS_H

#include "http.h"

int coolLEDsInit();
int coolLEDsRun();
int coolLEDsCleanup();
void wp_demo(struct HttpRequest *request);
int liquidAGCInit();
int liquidAGCRun();
int liquidAGCCleanup();
// int compassInit();
// int compassRun();
// int compassCleanup();

#endif
