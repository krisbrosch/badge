#include <stdint.h>
#include "driverlib.h"
#include "rgbled.h"
#include "uart.h"
#include "wifi.h"
#include "http.h"
#include "agc.h"
#include "timing.h"
#include "leddemos.h"
#include "sercli.h"

int oldDemoInit() {
  int i;
  wifi_start();

  for(i = 0; i < LEDCOUNT; i++) {
    ledData[i] = 0;
  }

  ledData[0] = 0x000000;
  ledData[1] = 0x000040;
  ledData[2] = 0x004000;
  ledData[3] = 0x400000;
  ledData[4] = 0x004040;
  ledData[5] = 0x400040;
  ledData[6] = 0x404000;
  ledData[7] = 0x404040;

  return 0;
}

int oldDemoRun() {
  int i;
  static uint32_t lastTime = 0;
  uint32_t dt;
  uint32_t temp1, temp2;
  http_loop();

  dt = get_ms() - lastTime;
  if(dt > 100) {
    lastTime += dt;
    temp1 = ledData[0];
      for(i = 1; i < LEDCOUNT; i++) {
      temp2 = ledData[i];
      ledData[i] = temp1;
      temp1 = temp2;
    }
    ledData[0] = temp1;
    sendLEDData();
    P2OUT ^= 3;
  }
  return 0;
}

int oldDemoCleanup() {
  wifi_off();
  return 0;
}

struct BadgeMode {
  int (*init)();
  int (*run)();
  int (*cleanup)();
};

struct BadgeMode badgeModes[] = {
  //{&oldDemoInit, &oldDemoRun, &oldDemoCleanup},
  {&coolLEDsInit, &coolLEDsRun, &coolLEDsCleanup},
  {&liquidAGCInit, &liquidAGCRun, &liquidAGCCleanup},
  //{&compassInit, &compassRun, &compassCleanup},
  {&serialCliInit, &serialCliRun, &serialCliCleanup},
};

int main(void) 
{
  int mode, nmodes, btnstatus;

  MAP_WDT_A_holdTimer();

  // Recommended values for main clock at 48MHz
  MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
  MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
  MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

  // Set main clock to DCO at 48MHz
  MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
  MAP_CS_initClockSignal(CS_MCLK,   CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  // Set HSMCLK and SMCLK to follow MCLK at 48MHz
  MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  MAP_CS_initClockSignal(CS_SMCLK,  CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

  // Enable SysTick counter
  MAP_SysTick_enableModule();
  MAP_SysTick_setPeriod(0x01000000);

  P2DIR |= 7;
  P2OUT &= ~(7);
  P2OUT |= 2;

  P1DIR &= 0xED;
  P1REN |= 0x12;
  P1OUT |= 0x12;

  timing_init();
  initLEDs();
  uart_init();
  wifi_init();
  i2c_init();

  ser_print("Hello, world!\r\n");

  btnstatus = 1;
  mode = 0;
  nmodes = sizeof(badgeModes) / sizeof(struct BadgeMode);
  if(badgeModes[mode].init != NULL)
    badgeModes[mode].init();
  while(1) {
    if(((P1IN & 2) == 0) && (btnstatus != 0)) {
      // Debounce
      delay_ms(10);
      btnstatus = 0;
      // cycle mode
      if(badgeModes[mode].cleanup != NULL)
        badgeModes[mode].cleanup();
      mode = (mode + 1) % nmodes;
      if(badgeModes[mode].init != NULL)
        badgeModes[mode].init();
    } else {
      btnstatus = (P1IN & 2);
    }
    badgeModes[mode].run();
  }

  return 0;
}
