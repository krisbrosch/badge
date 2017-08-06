#ifndef BATTTEMP_H
#define BATTTEMP_H

#include "driverlib.h"

#define BATTERY_GPIO_PORT GPIO_PORT_P5
#define BATTERY_GPIO_PIN GPIO_PIN5

int32_t battery_getPercent();

#endif
