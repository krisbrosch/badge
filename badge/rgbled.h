#ifndef RGBLED_H
#define RGBLED_H

#include <stdint.h>
#include "msp.h"

#define LEDCOUNT 12
#define LEDPORT P1OUT
#define LEDPORTDIR P1DIR
#define LEDPIN 7

extern uint32_t ledData[];

void initLEDs();
uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);
void sendLEDData();

#endif
