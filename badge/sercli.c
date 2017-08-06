
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sercli.h"
#include "uart.h"
#include "rgbled.h"
#include "wifi.h"
#include "driverlib.h"
#include "timing.h"

#define NUM_CLI_PARAMS 3
#define COMMAND_LINE_LENGTH 256

uint32_t ledWaitColor = 0x000201;

void setLeds(uint32_t color) {
  int i;
  for(i = 0; i < LEDCOUNT; i++) {
    ledData[i] = color;
  }
  sendLEDData();
}

struct CliCommand {
  char *command;
  int (*run)();
  char *helpText;
};

int command_help();
int command_leds();
int command_wifi_serial();

struct CliCommand commands[] = {
  {"leds", command_leds, "Set color of all leds"},
  {"wifi_serial", command_wifi_serial, "Pass through serial to wifi."},
  {"help", command_help, "This help"},
};

int command_help() {
  int i;
  ser_print("\r\n");
  for(i = 0; i < sizeof(commands) / sizeof(struct CliCommand); i++) {
    ser_print(" ");
    ser_print(commands[i].command);
    ser_print("\t");
    ser_print(commands[i].helpText);
    ser_print("\r\n");
  }
  ser_print("\r\n");
  return 0;
}

int command_leds(char *args[]) {
  if(args[0] == NULL) {
    return -1;
  }
  ledWaitColor = strtoul(args[0], NULL, 16);
  return 0;
}

int command_wifi_serial(char *args[]) {
  // TODO: This works ok, but it deserves a better solution
  volatile void *BTNS = &P1IN;
  uint32_t btnmask = 0x12;
  volatile void *TX1 = &BITBAND_PERI(P3OUT, 3);
  volatile void *TX2 = &BITBAND_PERI(P1OUT, 3);
  volatile void *RX1 = &BITBAND_PERI(P3IN, 2);
  volatile void *RX2 = &BITBAND_PERI(P1IN, 2);
  volatile void *RST = &BITBAND_PERI(P3OUT, 5);
  volatile void *GP0 = &BITBAND_PERI(P3OUT, 7);
  volatile void *RTS = &BITBAND_PERI(P8IN, 7);
  volatile void *DTR = &BITBAND_PERI(P8IN, 6);
  uint8_t temp1 = 0, temp2 = 0, temp3 = 0;

  ser_print("\r\n-- Starting ESP8266 Serial Passthrough Mode - press button to break --\r\n\r");
  uart_txByteSync(SER_MODULE, '\n');
  uart_disable();

  P8DIR &= 0x3f;
  WIFI_PORT_DIR |= (1 << WIFI_GPIO0_PIN);
  WIFI_PORT |= (1 << WIFI_GPIO0_PIN);
  //WIFI_PORT_DIR |= (1 << WIFI_CH_PD_PIN);
  WIFI_PORT |= (1 << WIFI_CH_PD_PIN);

  MAP_Interrupt_disableMaster();

  asm volatile (
    "wifi_serial_loop:\n"
    "ldrb %[temp1], [%[RX2]]\n"
    "strb %[temp1], [%[TX1]]\n"
    "ldrb %[temp1], [%[RX1]]\n"
    "strb %[temp1], [%[TX2]]\n"
    "ldrb %[temp1], [%[RTS]]\n"
    "ldrb %[temp2], [%[DTR]]\n"
    "movs %[temp3], 1\n"
    "eors %[temp3], %[temp3], %[temp1]\n"
    "orrs %[temp3], %[temp3], %[temp2]\n"
    "strb %[temp3], [%[RST]]\n"
    "movs %[temp3], 1\n"
    "eors %[temp3], %[temp3], %[temp2]\n"
    "orrs %[temp3], %[temp3], %[temp1]\n"
    "strb %[temp3], [%[GP0]]\n"
    "ldrb %[temp1], %[BTNS]\n"
    "tst %[temp1], %[btnmask]\n"
    "beq wifi_serial_loop\n"
    :
    : [TX1] "r" (TX1), [TX2] "r" (TX2), [RX1] "r" (RX1), [RX2] "r" (RX2), [RST] "r" (RST), [GP0] "r" (GP0), [RTS] "r" (RTS), [DTR] "r" (DTR), [temp1] "r" (temp1), [temp2] "r" (temp2), [temp3] "r" (temp3), [BTNS] "r" (BTNS), [btnmask] "r" (btnmask)
    :
  );
  MAP_Interrupt_enableMaster();

  WIFI_PORT |= (1 << WIFI_RST_PIN);
  WIFI_PORT |= (1 << WIFI_GPIO0_PIN);
  WIFI_PORT &= ~(1 << WIFI_CH_PD_PIN);
  
  uart_init();
  ser_print("\r\n\r\n-- ESP8266 Passthrough Done. --\r\n\r\n");
  return 0;
}

int serialCliInit() {
  setLeds(ledWaitColor);
  ser_print("\r\nBadge CLI Started\r\n\r\n> ");
  return 0;
}

int serialCliCleanup() {
  ser_print("\r\n\r\nBadge CLI Ended\r\n\r\n");
  return 0;
}

int serialCliRun() {
  char cmdbuf[COMMAND_LINE_LENGTH];
  int i, found;
  char *cmd;
  char *args[NUM_CLI_PARAMS];
  char *saveptr;

  if(ser_peek() != -1) {
    setLeds(0x020000);

    ser_getLine(cmdbuf, COMMAND_LINE_LENGTH);
    cmd = strtok_r(cmdbuf, " \t", &saveptr);
    for(i = 0; i < NUM_CLI_PARAMS; i++) {
      args[i] = strtok_r(NULL, " \t", &saveptr);
    }

    found = 0;
    for(i = 0; i < sizeof(commands) / sizeof(struct CliCommand); i++) {
      if( strcmp(cmd, commands[i].command) == 0 ) {
        found = 1;
        commands[i].run(args);
        break;
      }
    }
    if( (!found) && (strlen(cmd) > 0) ) {
      ser_print("Unknown command\r\n");
    }
    ser_print("> ");
    setLeds(ledWaitColor);
  }
  return 0;
}

