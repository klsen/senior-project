/*
 * Header file for using battery with ADC to read voltage and estimate capacity.
 */

#ifndef BATTERY_H
#define BATTERY_H

#include "stm32l0xx_hal.h"
#include "timers.h"
#include "TFT_display.h"
#include "user_interface.h"
#include <stdio.h>		// for sprintf

// defines for gpios used for enable lines
#define POWER_SUPPLY_ENABLE_PORT 	GPIOC
#define POWER_SUPPLY_ENABLE_PIN 	GPIO_PIN_4
#define ADC_DIVIDER_PORT 			GPIOC
#define ADC_DIVIDER_PIN 			GPIO_PIN_5
// note: ADC input on PA0

#define NUM_SAMPLES 	10

volatile uint8_t canSampleBattery;		// used in timer
uint8_t battPercentage;
uint8_t bState;

enum batteryState {
	batteryNormal,
	batteryLow,
	batteryReallyLow
	// no state for 0% battery because system will have no power at that point
};

// main function that runs samples and estimates battery percentage
void batteryManager(ADC_HandleTypeDef *hadc, SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *backlightTim);

// functions for entering and exiting low power mode
void startLowPowerMode(TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *backlightTim);
void stopLowPowerMode(TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *backlightTim);

// runs ADC and compares voltage read to an array to estimate capacity
uint8_t getBatteryPercentage(ADC_HandleTypeDef *hadc);

// looks into array and returns an index
// array represents battery voltage vs capacity curve, but only
// looking into linear-like section. sorted by largest to smallest,
// so it returns when val > array at index
uint8_t search(float val);

// small tester
void testBatteryCalculator(ADC_HandleTypeDef *hadc, SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *backlightTim);

#endif
