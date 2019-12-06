// file for using hardware timers for various things

#ifndef TIMERS_H
#define TIMERS_H

#include "stm32l0xx_hal.h"
#include "clocks.h"
#include "navigation.h"
#include "main.h"

#define MOTOR_PORT 	GPIOB
#define MOTOR_PIN 	GPIO_PIN_0

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

// TEST ALL THESE TIMERS TO SEE IF THEY'RE BEING CLOCKED LIKE YOU THINK THEY ARE

volatile uint32_t timerCounter;
volatile uint32_t stopwatchCounter;		// 16-bit can't hold a day's worth of seconds

// ---- important timer functions ----
// changed: timer and stopwatch use TIM21 CH1 and CH2
// timer uses LPTIM1
//void runTimer(LPTIM_HandleTypeDef *lptim);
//void pauseTimer(LPTIM_HandleTypeDef *lptim);
//void stopTimer(LPTIM_HandleTypeDef *lptim);
void runTimer(TIM_HandleTypeDef *htim);
void pauseTimer(TIM_HandleTypeDef *htim);
void stopTimer(TIM_HandleTypeDef *htim);

// stopwatch uses TIM21 (LSE counter)
void runStopwatch(TIM_HandleTypeDef *htim);
void pauseStopwatch(TIM_HandleTypeDef *htim);
void clearStopwatch(TIM_HandleTypeDef *htim);

void runTimerStopwatchBase(TIM_HandleTypeDef *htim);
void stopTimerStopwatchBase(TIM_HandleTypeDef *htim);
// ---- important timer functions ----

// ---- motor PWM and other functions ----
// adc sampler uses TIM22 (LSE counter)
// samples battery on set timing of about a sample/min
// changed: sampler uses TIM22
void runADCSampler(TIM_HandleTypeDef *htim);

// motor uses TIM6 (basic counter clocked on HSI)
// backlight PWM uses TIM7 (basic counter clocked on HSI)
// changed: motor and backlight use LSE TIM2, backlight uses CH1
void runMotor(TIM_HandleTypeDef *htim);
void runDisplayBacklight(TIM_HandleTypeDef *htim);
void stopDisplayBacklight(TIM_HandleTypeDef *htim);

// should run on startup with tft_display and stop when motor and backlight are unused
// flags?
void runMotorBacklightBase(TIM_HandleTypeDef *htim);
void stopMotorBacklightBase(TIM_HandleTypeDef *htim);

// sleep timer on LSE timer TIM2? would be used for going to sleep mode when system is idle for a while
// ---- end of other functions ----

#endif
