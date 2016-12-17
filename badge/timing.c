#include "driverlib.h"
#include "timing.h"

// Millisecond counter
volatile uint32_t msctr = 0;

void timing_init() {
  // SysTick will count down at full clock speed (48MHz)
  // The interrupt will fire every 48000 clocks (1KHz, or every 1mS)
  MAP_SysTick_enableModule();
  MAP_SysTick_setPeriod(48000);
  MAP_SysTick_enableInterrupt();
}

uint32_t get_ms() {
  return msctr;
}

void delay_ms(uint32_t n) {
  uint32_t start = msctr;
  while(msctr - start < n);
}

void sysTick_isr(void) {
  msctr++;
}

