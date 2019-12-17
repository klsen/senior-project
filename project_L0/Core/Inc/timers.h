/*
 * File for managing hardware timers.
 * L0 has 4 general purpose timers, 2 basic timers, and 1 low-power timer.
 *
 * Very much tied to project to control the hardware used for
 * running stopwatch, timer, LCD backlight, sampling period for battery voltage,
 * and software debouncing.
 */

#ifndef TIMERS_H
#define TIMERS_H

#include "stm32l0xx_hal.h"
#include "user_interface.h"
#include "clocks.h"
#include "battery.h"

// defines for motor enable gpio
#define MOTOR_PORT 	GPIOB
#define MOTOR_PIN 	GPIO_PIN_5

// volatile variables to hold a long-term counter for timer and stopwatch
// 16-bit can't hold a day's worth of seconds
volatile uint32_t timerCounter;
volatile uint32_t stopwatchCounter;

// ---- important timer functions ----
// timer and stopwatch use TIM21 CH1 and CH2
// assumes timers are configured to use LSE crystal with period set to 1s
void runTimer(TIM_HandleTypeDef *htim);
void pauseTimer(TIM_HandleTypeDef *htim);
void stopTimer(TIM_HandleTypeDef *htim);
void runStopwatch(TIM_HandleTypeDef *htim);
void pauseStopwatch(TIM_HandleTypeDef *htim);
void clearStopwatch(TIM_HandleTypeDef *htim);

// run time base so it's ready when we use output compare or PWM timer functions
// might not need base functions since HAL might be enabling them when running OC or PWM
void runTimerStopwatchBase(TIM_HandleTypeDef *htim);
void stopTimerStopwatchBase(TIM_HandleTypeDef *htim);
// ---- important timer functions ----

// ---- motor PWM and other functions ----
// sampler uses TIM22
// samples battery on set timing of about a sample/min
void runADCSampler(TIM_HandleTypeDef *htim);

// motor and backlight use LSE TIM2, backlight uses CH1, motor uses CH2 respectively
// uses timers configured to use LSE crystal with 0.5s period
void runMotor(TIM_HandleTypeDef *htim);
void stopMotor(TIM_HandleTypeDef *htim);
void runDisplayBacklight(TIM_HandleTypeDef *htim);
void changeDisplayBacklight(uint8_t intensity, TIM_HandleTypeDef *htim);
void stopDisplayBacklight(TIM_HandleTypeDef *htim);

// should run on startup with tft_display and stop when motor and backlight are unused
void runBacklightMotorBase(TIM_HandleTypeDef *htim);
void stopBacklightMotorBase(TIM_HandleTypeDef *htim);
// ---- end of other functions ----

#endif
