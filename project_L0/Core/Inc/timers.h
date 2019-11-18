// file for using hardware timers for various things

#ifndef TIMERS_H
#define TIMERS_H

#include "stm32l0xx_hal.h"
#include "clocks.h"
#include "navigation.h"

/*
 * for using timer hardware on l0
 * using for stopwatch, pwm, and timer of watch
 *
 * better to use rtc alarm instead for timer?
 * reuse 1 rtc alarm for both timer and alarm? not gonna work for case where alarm happens
 * before timer, but maybe we can check between values and shuffle accordingly.
 * gonna need to show how much time left on screen. using regular timer for that?
 *
 * regular timer for stopwatch
 * pwm timer for backlight
 * also timer to time motor signal? if hal_delay doesnt work?
 *
 * what hardware gets shut off in microcontroller low power mode?
 * timers i need to save?
 *
 * time to read
 */

uint32_t stopwatchCNT;		// 16-bit can't hold a day's worth of seconds

// ---- stopwatch functions ----
void runStopwatch(LPTIM_HandleTypeDef *hlptim);
void pauseStopwatch(LPTIM_HandleTypeDef *hlptim);
void clearStopwatch(LPTIM_HandleTypeDef *hlptim);
// ---- end of stopwatch functions ----

// ---- timer functions ----
void runTimerDisplay(TIM_HandleTypeDef *htim);
// ---- end of timer functions ----

// ---- motor PWM and other functions ----
void runMotor(TIM_HandleTypeDef *htim);
// ---- end of other functions ----

#endif
