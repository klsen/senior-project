#ifndef BATTERY_H
#define BATTERY_H

#include "stm32l0xx_hal.h"
#include "timers.h"
#include "TFT_display.h"

#define POWER_SUPPLY_ENABLE_PORT 	GPIOC
#define POWER_SUPPLY_ENABLE_PIN 	GPIO_PIN_4
#define ADC_DIVIDER_PORT 			GPIOC
#define ADC_DIVIDER_PIN 			GPIO_PIN_5

#define NUM_SAMPLES 	10

/*
 * define gpios for enable lines, channel and pin
 * calculate percentage:
 *
 * ADC channel 0 input on PA0
 */
volatile uint8_t canSampleBattery;		// used in timer
uint8_t battPercentage;		// might need to be somewhat global for display

enum batteryState {
	batteryNormal,
	batteryLow,
	batteryReallyLow
	// no state for 0% battery because system will have no power at that point
};

void batteryManager(ADC_HandleTypeDef *hadc);
uint8_t getBatteryPercentage(ADC_HandleTypeDef *hadc);
uint8_t search(float val);
void testBatteryCalculator(ADC_HandleTypeDef *hadc, SPI_HandleTypeDef *hspi);

#endif
