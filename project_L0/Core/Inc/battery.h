#ifndef BATTERY_H
#define BATTERY_H

#include "stm32l0xx_hal.h"
#include "timers.h"

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

uint8_t calculateBatteryPercentage(ADC_HandleTypeDef *hadc);
uint8_T search(float32_t val);
void testBatteryCalculator(ADC_HandleTypeDef *hadc);

#endif
