#include "rgbled.h"
#include "driverlib.h"

uint32_t ledData[LEDCOUNT];

void initLEDs() {
  int i;
  LEDPORTDIR |= (1 << LEDPIN);
  for(i = 0; i < LEDCOUNT; i++) {
    ledData[i] = 0;
  }
}

uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b);
}

void sendLEDData() {
  uint32_t ledctr;
  int32_t bitctr;
  volatile uint8_t *port = &(BITBAND_PERI(LEDPORT,LEDPIN));
  uint32_t data;
  uint8_t setpin = 1;
  uint8_t clrpin = 0;
  static uint32_t lastCount = 0xffffffff;

  // This article has a great exploration of the flexibility of timings:
  // https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/

  // WS2812B timings converted to clock ranges at 48MHz
  // T0H  250 - 550     12 - 26
  // T1H  650 - 950     32 - 45
  // T0L  700 - 1000    34 - 48
  // T1L  300 - 600     15 - 28

  // Experimental measured timings of this code:
  // T0H 340
  // T1H 800
  // T0L 960
  // T1L 960

  // Experimentally, the strb instructions that set or clear the
  // output pin take anywhere from 2-15 clock cycles, presumably
  // due to the bus stalling the pipeline.

  // I don't have any nop's after clearing the pin; the rest of
  // the loop appears to take enough time to fill T0L and T1L

  // Ensure reset time has elapsed
  // Assuming SysTick is running at clock speed (48MHz)
  if(lastCount != 0xffffffff) {
    while(lastCount - SysTick->VAL < 2500);
  }

  for(ledctr = 0; ledctr < LEDCOUNT; ledctr++) {
    data = ledData[ledctr];
    data = ((data & 0x0000ff00) << 8) | ((data & 0x00ff0000) >> 8) | (data & 0x000000ff);
    for(bitctr = 23; bitctr >=0; bitctr--) {
      if(data & 0x00800000) { // 1
        asm volatile (
          "strb %[setpin], [%[port]]\n"
          "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"    // 16
          "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"                                            // 8
          "strb %[clrpin], [%[port]]\n"                                                         // 2-15
          :
          : [port] "r" (port), [setpin] "r" (setpin), [clrpin] "r" (clrpin)
          :
        );
      } else {                // 0
        asm volatile (
          "cpsid i\n"
          "strb %[setpin], [%[port]]\n"
          "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"                                  // 10
          "strb %[clrpin], [%[port]]\n"                                                         // 2-15
          "cpsie i\n"
          :
          : [port] "r" (port), [setpin] "r" (setpin), [clrpin] "r" (clrpin)
          :
        );
      }
      data = data << 1;
    }
  }

  lastCount = SysTick->VAL;
  if(lastCount < 2500) {
    lastCount += SysTick->LOAD;
  }
}

