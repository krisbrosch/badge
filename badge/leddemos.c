#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "driverlib.h"
#include "rgbled.h"
#include "timing.h"
#include "leddemos.h"
#include "wifi.h"
#include "http.h"
#include "agc.h"
#include "uart.h"

// One quarter of a sin wave
static uint8_t sintable[] = {
128, 129, 131, 132, 134, 135, 137, 138, 140, 142, 143,
145, 146, 148, 149, 151, 152, 154, 156, 157, 159, 160,
162, 163, 165, 166, 168, 169, 171, 172, 174, 175, 176,
178, 179, 181, 182, 184, 185, 186, 188, 189, 191, 192,
193, 195, 196, 197, 199, 200, 201, 202, 204, 205, 206,
207, 209, 210, 211, 212, 213, 215, 216, 217, 218, 219,
220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
231, 232, 233, 234, 235, 236, 236, 237, 238, 239, 240,
240, 241, 242, 243, 243, 244, 245, 245, 246, 246, 247,
247, 248, 249, 249, 250, 250, 250, 251, 251, 252, 252,
252, 253, 253, 253, 254, 254, 254, 254, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255
};

uint32_t a_p;
uint32_t a_f;
uint32_t a_speed;
uint8_t a_r, a_g, a_b;

uint32_t b_p;
uint32_t b_f;
uint32_t b_speed;
uint8_t b_r, b_g, b_b;

uint32_t redburst;

uint32_t wifiMode;

uint32_t lastTime;

uint8_t square(uint32_t x) {
  if((x & 0xFFFF) > 0x7fff) {
    return 0xff;
  }
  return 0;
}

uint8_t sinx(uint32_t ix) {
  uint32_t x;
  x = (ix & 0xFFFF) >> 7;
  if(x < 128)
    return sintable[x];
  if(x < 256)
    return sintable[127-(x-128)];
  if(x < 384)
    return 255 - sintable[x-256];
  return 255 - sintable[127-(x-384)];
}

int coolLEDsInit() {
  wifiMode = 0;

  a_r = 0x7f;
  a_g = 0x00;
  a_b = 0x7f;
  a_speed = 40;
  a_p = 0;
  a_f = 1;

  b_r = 0x00;
  b_g = 0x33;
  b_b = 0x66;
  b_speed = 60;
  b_p = 0;
  b_f = 2;

  redburst = 0;
  lastTime = get_ms();
  return 0;
}

int coolLEDsRun() {
  int i;
  uint32_t dt;
  uint32_t x;
  uint8_t a_value, b_value;
  uint8_t a_cr, a_cg, a_cb, b_cr, b_cg, b_cb;
  uint32_t total_color;
  uint32_t burstdecay;
  static uint32_t lastReleased = 0xffffffff;

  if(wifiMode) {
    http_loop();
  }

  dt = get_ms() - lastTime;
  lastTime += dt;

  a_p -= a_speed * dt;
  a_p = a_p & 0xffff;

  b_p -= b_speed * dt;
  b_p = b_p & 0xffff;

  // Exponential decay; tau = 200mS
  burstdecay = 227 * (redburst >> 16) * dt;
  if(burstdecay > redburst)
    redburst = 0;
  else
    redburst -= burstdecay;

  if((P1IN & 0x10) == 0) {
    // Button 2 initiates burst
    // TODO: Accel. initiates burst
    redburst = 0xff000000;
    // WiFi toggle
    if(get_ms() - lastReleased > 1500) {
      if(wifiMode) {
        wifiMode = 0;
        wifi_off();
        for(i = 0; i < LEDCOUNT; i++) {
          ledData[i] = 0x7f7f00;
        }
        sendLEDData();
        delay_ms(500);
      } else {
        wifiMode = 1;
        for(i = 0; i < LEDCOUNT; i++) {
          ledData[i] = 0x007f00;
        }
        sendLEDData();
        delay_ms(500);
        wifi_start();
      }
    }
  } else {
    lastReleased = get_ms();
  }

  x = 0;
  for(i = 0; i < LEDCOUNT; i++) {
    a_value = sinx(x * a_f + a_p);
    b_value = sinx(x * b_f + b_p);
    a_cr = ((uint32_t)a_value * (uint32_t)a_r) >> 8;
    a_cg = ((uint32_t)a_value * (uint32_t)a_g) >> 8;
    a_cb = ((uint32_t)a_value * (uint32_t)a_b) >> 8;
    b_cr = ((uint32_t)b_value * (uint32_t)b_r) >> 8;
    b_cg = ((uint32_t)b_value * (uint32_t)b_g) >> 8;
    b_cb = ((uint32_t)b_value * (uint32_t)b_b) >> 8;
    total_color = rgb((a_cr / 3) + (b_cr / 3) + ((uint8_t)(redburst >> 24)/3),
                      (a_cg / 3) + (b_cg / 3),
                      (a_cb / 3) + (b_cb / 3));
    ledData[i] = total_color;
    x += (65536/LEDCOUNT);
  }

  sendLEDData();
  return 0;
}

int coolLEDsCleanup() {
  if(wifiMode)
    wifi_off();
  return 0;
}

const char *cooldemohtml =
"<html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"<style>body { background-color: #333333 }\n"
"* { color: #007fff; font-family: \"Courier New\", Courier, monospace; font-weight: bold }\n"
"h1 { color: #ffaa00; text-align: center } input {width:100%%}\n"
".r {display: table; width: 100%%} .l {display: table-cell; width: 10%%} .c {display: table-cell}\n"
"</style><script>function go() { document.forms[0].submit(); }</script>"
"<body><form action=\"/demo\" method=\"get\"><h1>BADGE</h1>"
"<div class=\"r\"><div class=\"l\">Color1:</div><div class=\"c\">"
"<input name=\"ac\" type=\"color\" value=\"#%02x%02x%02x\" onchange=\"go();\" /></div></div>"
"<div class=\"r\"><div class=\"l\">F1:</div><div class=\"c\">"
"<input name=\"af\" type=\"range\" min=\"0\" max=\"12\" step=\"1\" value=\"%d\" onchange=\"go();\" /></div></div>"
"<div class=\"r\"><div class=\"l\">S1:</div><div class=\"c\">"
"<input name=\"as\" type=\"range\" min=\"0\" max=\"300\" step=\"1\" value=\"%d\" onchange=\"go();\" /></div></div>"
"<div class=\"r\"><div class=\"l\">Color2:</div><div class=\"c\">"
"<input name=\"bc\" type=\"color\" value=\"#%02x%02x%02x\" onchange=\"go();\" /></div></div>"
"<div class=\"r\"><div class=\"l\">F2:</div><div class=\"c\">"
"<input name=\"bf\" type=\"range\" min=\"0\" max=\"12\" step=\"1\" value=\"%d\" onchange=\"go();\" /></div></div>"
"<div class=\"r\"><div class=\"l\">S2:</div><div class=\"c\">"
"<input name=\"bs\" type=\"range\" min=\"0\" max=\"300\" step=\"1\" value=\"%d\" onchange=\"go();\" /></div></div>"
"</form></html>";

void wp_demo(struct HttpRequest *request) {
  char buf[2000];
  char *p;

  p = http_getUrlParam(request, "ac");
  if(p) {
    a_r = (hexToNibble(p[1]) << 4) | hexToNibble(p[2]);
    a_g = (hexToNibble(p[3]) << 4) | hexToNibble(p[4]);
    a_b = (hexToNibble(p[5]) << 4) | hexToNibble(p[6]);
  }
  p = http_getUrlParam(request, "bc");
  if(p) {
    b_r = (hexToNibble(p[1]) << 4) | hexToNibble(p[2]);
    b_g = (hexToNibble(p[3]) << 4) | hexToNibble(p[4]);
    b_b = (hexToNibble(p[5]) << 4) | hexToNibble(p[6]);
  }
  p = http_getUrlParam(request, "af");
  if(p) {
    a_f = atoi(p);
    if(a_f > 12)
      a_f = 12;
  }
  p = http_getUrlParam(request, "bf");
    if(p) {
    b_f = atoi(p);
    if(b_f > 12)
      b_f = 12;
  }
  p = http_getUrlParam(request, "as");
  if(p) {
    a_speed = atoi(p);
    if(a_speed > 300)
      a_speed = 300;
  }
  p = http_getUrlParam(request, "bs");
  if(p) {
    b_speed = atoi(p);
    if(b_speed > 300)
      b_speed = 300;
  }

  snprintf(buf, 2000, cooldemohtml, a_r, a_g, a_b, a_f, a_speed, b_r, b_g, b_b, b_f, b_speed);
  http_reply_ok(request, (uint8_t *)buf, strlen(buf));
}

//AGC DEMOS

uint8_t liquidLEDInt[LEDCOUNT];
uint8_t liquid_decay_delay = 2;
uint8_t sky_r = 0xdd;
uint8_t sky_g = 0x88;
uint8_t sky_b = 0x00;
uint8_t liquid_r = 0x00;
uint8_t liquid_g = 0x88;
uint8_t liquid_b = 0xdd;
uint8_t liquid_side = 3;

int liquidAGCInit(){
  int i;
  for (i = 0; i < LEDCOUNT; i++){
    ledData[i] = 0;
    liquidLEDInt[i] = 0;
  }
        lastTime = get_ms();
  sendLEDData();
  return 0;
}

int ledAtan_lookup(int32_t in, int32_t x){
  if(x > 0) {
    if(in < -244583) return 9;
    if(in < -65536) return 8;
    if(in < -17560) return 7;
    if(in < 17560) return 6;
    if(in < 65535) return 5;
    if(in < 244583) return 4;
    return 3;
  } else {
    if(in < -244583) return 3;
    if(in < -65536) return 2;
    if(in < -17560) return 1;
    if(in < 17560) return 0;
    if(in < 65535) return 11;
    if(in < 244583) return 10;
    return 9;
  }
  return 0;
}

int liquidAGCRun(){
  static uint32_t decayTimer = 0;
  uint32_t dt;
  uint16_t r, g, b;
  int i, ii;
  int16_t abs_lin_z;

  dt = get_ms() - lastTime;
  lastTime += dt;

  //read accelerometer x, y, z axes:
  int16_t lin_x = agc_read_sensor_axis(AGC_LINEAR_ACCELEROMETER_SENSOR, AGC_X_AXIS);
  ser_printf("lin_x = %i\r\n", lin_x);
  int16_t lin_y = agc_read_sensor_axis(AGC_LINEAR_ACCELEROMETER_SENSOR, AGC_Y_AXIS);
  ser_printf("lin_y = %i\r\n", lin_y);
  int16_t lin_z = agc_read_sensor_axis(AGC_LINEAR_ACCELEROMETER_SENSOR, AGC_Z_AXIS);
  ser_printf("lin_z = %i\r\n\r\n", lin_z);

  if(lin_z > 0)
    abs_lin_z = lin_z;
  else
    abs_lin_z = -lin_z;

  if (lin_x == 0) lin_x = 1;

  int32_t agc_angle_in = (lin_y << 16) / lin_x;
  int ledIndex = ledAtan_lookup(agc_angle_in, lin_x);


  decayTimer += dt;
  while(decayTimer > liquid_decay_delay) {
    decayTimer -= liquid_decay_delay;
    for(i = 0; i < LEDCOUNT; i++) {
      liquidLEDInt[i] = (liquidLEDInt[i] == 0) ? 0 : liquidLEDInt[i] - 1;
    }
  }

  liquidLEDInt[ledIndex] = 255;
  if(abs_lin_z < 8192) {
    liquid_side = 0;
  } else if(abs_lin_z < 12288) {
    liquid_side = 1;
  } else if(abs_lin_z < 20480) {
    liquid_side = 2;
  } else if(abs_lin_z < 28672) {
    liquid_side = 3;
  }
  for(i = 1; i <= liquid_side; i++) {
    liquidLEDInt[(ledIndex + i + LEDCOUNT) % LEDCOUNT] = 255;
    liquidLEDInt[(ledIndex - i + LEDCOUNT) % LEDCOUNT] = 255;
  }

  for (i = 0; i < LEDCOUNT; i++){
    ii = liquidLEDInt[i];
    r = (((uint16_t)sky_r * (uint16_t)(255-ii)) >> 9) + (((uint16_t)liquid_r * (uint16_t)ii) >> 9);
    g = (((uint16_t)sky_g * (uint16_t)(255-ii)) >> 9) + (((uint16_t)liquid_g * (uint16_t)ii) >> 9);
    b = (((uint16_t)sky_b * (uint16_t)(255-ii)) >> 9) + (((uint16_t)liquid_b * (uint16_t)ii) >> 9);
    ledData[i] = rgb(r,g,b);
  }

  //ledData[ledIndex] = 0xAF25AF;
  sendLEDData();

  //how much delay needed?
  //delay_ms(100);
  delay_ms(10);
  return 0;
}

int liquidAGCCleanup(){
  return 0;
}


// int compassInit() {
//   int i;
//   for (i = 0; i < LEDCOUNT; i++) {
//     ledData[i] = 0;
//   }
//   sendLEDData();
//   return 0;
// }
// 
// int compassRun() {
//   int i, ledIndex;
//   int16_t mx, my, mz;
//   int32_t tana;
// 
// 
//   mx = agc_read_sensor_axis(AGC_MAGNETOMETER_SENSOR, AGC_X_AXIS);
//   my = agc_read_sensor_axis(AGC_MAGNETOMETER_SENSOR, AGC_Y_AXIS);
//   mz = agc_read_sensor_axis(AGC_MAGNETOMETER_SENSOR, AGC_Z_AXIS);
//   ser_printf("mag_x = %i\r\n", mx);
//   ser_printf("mag_y = %i\r\n", my);
//   ser_printf("mag_z = %i\r\n\r\n", mz);
// 
//   if (mx == 0) mx = 1;
//   tana = (my << 16) / mx;
//   ledIndex = ledAtan_lookup(tana, mx);
// 
//   for (i = 0; i < LEDCOUNT; i++) {
//     ledData[i] = 0;
//   }
//   ledData[ledIndex] = 0x101010;
//   sendLEDData();
// 
//   // ???
//   delay_ms(100);
// 
//   return 0;
// }
// 
// int compassCleanup() {
//   return 0;
// }


