#ifndef WIFI_H
#define WIFI_H

#include <stdlib.h>

#define WIFI_PORT P3OUT
#define WIFI_PORT_DIR P3DIR
#define WIFI_RST_PIN 5
#define WIFI_CH_PD_PIN 6
#define WIFI_GPIO0_PIN 7

int wifi_poll(uint32_t *rxlen, uint8_t *buf, size_t length);
void wifi_send(int channel, const uint8_t *data, uint32_t length);
void wifi_close(int channel);
void wifi_init();
//void wifi_reset();
void wifi_off();
void wifi_on();
void wifi_start();

#endif
