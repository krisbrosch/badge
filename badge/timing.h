#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

void timing_init();
uint32_t get_ms();
void delay_ms(uint32_t n);

#endif
