#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "wifi.h"
#include "driverlib.h"
#include "uart.h"
#include "timing.h"

#define WIFI_MODULE UART2_MODULE

// WARNING: Can't handle repeated characters in any of the strings!
int readUartUntil3(const char *stringa, const char *stringb, const char *stringc) {
  uint32_t ia, ib, ic, na, nb, nc;
  char c;
  na = (stringa == NULL) ? 0xffffffff : strlen(stringa);
  nb = (stringb == NULL) ? 0xffffffff : strlen(stringb);
  nc = (stringc == NULL) ? 0xffffffff : strlen(stringc);
  ia = 0; ib = 0; ic = 0;
  while((ia < na) && (ib < nb) && (ic < nc)) {
    c = uart_rxByteWait(WIFI_MODULE);
    ia = ((stringa != NULL) && (c == stringa[ia])) ? ia+1 : 0;
    ib = ((stringb != NULL) && (c == stringb[ib])) ? ib+1 : 0;
    ic = ((stringc != NULL) && (c == stringc[ic])) ? ic+1 : 0;
  }
  if((stringa != NULL) && (ia == na)) {
    return 0;
  } else if((stringb != NULL) && (ib == nb)) {
    return 1;
  }
  return 2;
}

void readUartUntil(const char *s) {
  readUartUntil3(s, NULL, NULL);
}

void readUartUntilDone() {
  readUartUntil3("OK\r\n", "ERROR\r\n", "ready\r\n");
}

void wifi_cmd(const char *s) {
  uart_txSync(WIFI_MODULE, (uint8_t *)s, strlen(s));
  readUartUntilDone();
}

// Note that buf must be global or static because wifi_poll writes to
// it across multiple calls
int wifi_poll(uint32_t *rxlen, uint8_t *buf, size_t length) {
  static char tmpbuf[20];
  static int step = 0;
  static uint32_t i = 0;
  static uint32_t channel;
  static uint32_t inlen;
  int32_t x;

  switch(step) {
    case 0:  // Looking for "+IPD,"
      do {
        x = uart_rxByte(WIFI_MODULE);
        if(x != -1) {
          i = ((int32_t)("+IPD,"[i]) == x) ? i + 1 : 0;
        }
      } while((x != -1) && (i < 5));
      if(i == 5) {
        step = 1;
        i = 0;
      }
      if(x == -1) {
        return -1;
      }
    case 1:  // Reading the channel ID number
      do {
        x = uart_rxByte(WIFI_MODULE);
        if((x != -1) && (i < sizeof(tmpbuf)))
          tmpbuf[i++] = x;
      } while((x != ',') && (x != -1));
      if(x == ',') {
        tmpbuf[i-1] = 0;
        channel = atoi(tmpbuf);
        step = 2;
        i = 0;
      } else if(x == -1) {
        return -1;
      }
    case 2:  // Reading the length
      do {
        x = uart_rxByte(WIFI_MODULE);
        if((x != -1) && (i < sizeof(tmpbuf)))
          tmpbuf[i++] = x;
      } while((x != ':') && (x != -1));
      if(x == ':') {
        tmpbuf[i-1] = 0;
        inlen = atoi(tmpbuf);
        step = 3;
        i = 0;
      } else if(x == -1) {
        return -1;
      }
    case 3:  // Reading the data
      do {
        x = uart_rxByte(WIFI_MODULE);
        if((x != -1) && (i < inlen)) {
          if(i < length) {
            buf[i] = x;
          }
          i++;
        }
      } while((x != -1) && (i < inlen));
      if(x == -1) {
        return -1;
      }
      if(i == inlen) {
        *rxlen = i;
        step = 0;
        i = 0;
        return channel;
      }
    default:
      step = 0;
      i = 0;
  }
  return -1;
}

void wifi_send(int channel, const uint8_t *data, uint32_t length) {
  char tmpbuf[60];

  snprintf(tmpbuf, sizeof(tmpbuf), "AT+CIPSEND=%d,%u\r\n", channel, (unsigned int)length);
  uart_tx(WIFI_MODULE, (uint8_t *)tmpbuf, strlen(tmpbuf));
  if(readUartUntil3("\r\n> ", "ERROR\r\n", NULL) == 1) {
    return;
  }

  uart_tx(WIFI_MODULE, data, length);
  readUartUntilDone();
}

void wifi_close(int channel) {
  char tmpbuf[50];
  snprintf(tmpbuf, sizeof(tmpbuf), "AT+CIPCLOSE=%d\r\n", channel);
  wifi_cmd(tmpbuf);
}

void wifi_init() {
  // It doesn't seem to like to boot properly while GPIO0 is held high, but it's fine when we don't assert the pin
  WIFI_PORT_DIR &= ~(1 << WIFI_GPIO0_PIN);
  WIFI_PORT_DIR |= (1 << WIFI_RST_PIN) | (1 << WIFI_CH_PD_PIN);
  WIFI_PORT |= (1 << WIFI_CH_PD_PIN);
  WIFI_PORT |= (1 << WIFI_RST_PIN);
  delay_ms(100);
  wifi_off();
}

// TODO:
// void wifi_reset() {
// }

void wifi_off() {
  WIFI_PORT &= ~(1 << WIFI_CH_PD_PIN);
  delay_ms(100);
}

void wifi_on() {
  WIFI_PORT |= (1 << WIFI_CH_PD_PIN);
  delay_ms(100);
}

void wifi_startAP() {
  uint32_t n = 0;
  char mac[18];
  char startAP[100];
  const char *startAPt = "AT+CWSAP_CUR=\"badge_XXXX\",\"\",1,0\r\n";
  const char *getMAC = "AT+CIPAPMAC_CUR?\r\n";
  memcpy(startAP, startAPt, strlen(startAPt));
  uart_txSync(WIFI_MODULE, (uint8_t *)getMAC, strlen(getMAC));
  readUartUntil("+CIPAPMAC_CUR:\"");
  while(n < 17) {
    n += uart_rx(WIFI_MODULE, (uint8_t *)(&mac[n]), 17-n);
  }
  readUartUntil("OK\r\n");
  mac[17] = 0;
  startAP[20] = mac[12];
  startAP[21] = mac[13];
  startAP[22] = mac[15];
  startAP[23] = mac[16];
  wifi_cmd(startAP);
}

void wifi_start() {
  wifi_on();
  // empty out potential previous "ready"
  while(uart_rxByte(WIFI_MODULE) != -1);
  // TODO: Make some of this optional/configurable? Move it out of this fn?
  uart_txSync(WIFI_MODULE, (uint8_t *)"AT+RST\r\n", 8);
  readUartUntil("ready\r\n");
  wifi_cmd("ATE0\r\n");
  wifi_cmd("AT+CWMODE_CUR=2\r\n");
  wifi_startAP();
  wifi_cmd("AT+CWDHCP_CUR=0,1\r\n");
  wifi_cmd("AT+CIPMODE=0\r\n");
  wifi_cmd("AT+CIPMUX=1\r\n");
  wifi_cmd("AT+CIPSERVER=1,80\r\n");
}

