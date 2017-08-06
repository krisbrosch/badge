#include "batttemp.h"

#define BATTTEMP_BATTERY 0
#define BATTTEMP_TEMP 1

int32_t batttemp_singleSample(int mode) {
  int32_t value;

  MAP_ADC14_enableModule();
  if(mode == BATTTEMP_BATTERY) {
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_64, ADC_DIVIDER_8, ADC_NOROUTE);
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(BATTERY_GPIO_PORT, BATTERY_GPIO_PIN, GPIO_TERTIARY_MODULE_FUNCTION);
  } else {
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_64, ADC_DIVIDER_8, ADC_TEMPSENSEMAP);
  }
  MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
  MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, false);
  MAP_ADC14_setResolution(ADC_12BIT);
  MAP_ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
  MAP_ADC14_enableConversion();
  MAP_ADC14_toggleConversionTrigger();

  while(MAP_ADC14_isBusy());

  value =  (int32_t)MAP_ADC14_getResult(ADC_MEM0);

  MAP_ADC14_disableModule();

  return value;
}

int32_t battery_getPercent() {
  int32_t value;

  value = batttemp_singleSample(BATTTEMP_BATTERY);

  value = value >> 2;
  value = value - 573;
  if(value < 0) value = 0;
  value = value * 10000;
  value = value / 17067;
  if(value > 100) value = 100;

  return value;
}
