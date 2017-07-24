
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
  ser_print("\r\n-- Starting ESP8266 Serial Passthrough Mode - press button to break --\r\n\r");
  uart_txByteSync(SER_MODULE, '\n');
  uart_disable();

  WIFI_PORT |= (1 << WIFI_CH_PD_PIN);

  // Either button breaks
  while( (P1IN & 0x12) == 0x12 ) {
    // This is an experimental hack, but it seems to work well enough
    // TODO: add RTS/DTR or seperate bootloader mode?
    if(P1IN & 0x4) {
      P3OUT |= 0x8;
    } else {
      P3OUT &= ~0x8;
    }
    if(P3IN & 0x4) {
      P1OUT |= 0x8;
    } else {
      P1OUT &= ~0x8;
    }
  }

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

