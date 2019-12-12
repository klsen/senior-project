// file for using hardware timers for various things

#ifndef TIMERS_H
#define TIMERS_H

#include "user_interface.h"
#include "stm32l0xx_hal.h"
#include "clocks.h"
#include "battery.h"
#include "main.h"

#define MOTOR_PORT 	GPIOB
#define MOTOR_PIN 	GPIO_PIN_0

volatile uint32_t timerCounter;
volatile uint32_t stopwatchCounter;		// 16-bit can't hold a day's worth of seconds

// ---- important timer functions ----
// timer and stopwatch use TIM21 CH1 and CH2
void runTimer(TIM_HandleTypeDef *htim);
void pauseTimer(TIM_HandleTypeDef *htim);
void stopTimer(TIM_HandleTypeDef *htim);
void runStopwatch(TIM_HandleTypeDef *htim);
void pauseStopwatch(TIM_HandleTypeDef *htim);
void clearStopwatch(TIM_HandleTypeDef *htim);

void runTimerStopwatchBase(TIM_HandleTypeDef *htim);
void stopTimerStopwatchBase(TIM_HandleTypeDef *htim);
// ---- important timer functions ----

// ---- motor PWM and other functions ----
// sampler uses TIM22
// samples battery on set timing of about a sample/min
void runADCSampler(TIM_HandleTypeDef *htim);

// motor and backlight use LSE TIM2, backlight uses CH1, motor uses CH2 respectively
void runMotor(TIM_HandleTypeDef *htim);
void stopMotor(TIM_HandleTypeDef *htim);
void runDisplayBacklight(TIM_HandleTypeDef *htim);
void changeDisplayBacklight(uint8_t intensity, TIM_HandleTypeDef *htim);
void stopDisplayBacklight(TIM_HandleTypeDef *htim);

// should run on startup with tft_display and stop when motor and backlight are unused
// flags?
void runBacklightMotorBase(TIM_HandleTypeDef *htim);
void stopBacklightMotorBase(TIM_HandleTypeDef *htim);

// sleep timer on LSE timer TIM2? would be used for going to sleep mode when system is idle for a while
// ---- end of other functions ----

#endif
