/*
 * File for managing and drawing user interface for the watch project.
 * The user can run clock, timer, alarm, and stopwatch functions on
 * a display, and 4 buttons are used for user input.
 *
 * Assumes this set of hardware is available:
 *   built-in RTC
 *   hardware timers
 *   TFT display that uses SPI
 *
 * Graphics are built using the functions in TFT_display.h library.
 */

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "stm32l0xx_hal.h"
#include "TFT_display.h"
#include "clocks.h"
#include "timers.h"
#include "main.h"
#include <stdlib.h>		// for malloc
#include <stdio.h>		// for sprintf

// buttons connected to gpio external interrupts
// callback ignores ports, so only pin number is needed.
// uses HAL gpio pin number defines
#define BUTTON1 	GPIO_PIN_2
#define BUTTON2 	GPIO_PIN_13
#define BUTTON3 	GPIO_PIN_14
#define BUTTON4 	GPIO_PIN_15

// defines for gpios used for controlling LEDs. helps with debugging
#define LED1_PORT 	GPIOB
#define LED1_PIN	GPIO_PIN_4
#define LED2_PORT 	GPIOA
#define LED2_PIN	GPIO_PIN_10
#define LED3_PORT 	GPIOB
#define LED3_PIN	GPIO_PIN_3

// defines to help with flags
#define NUM_FACES 			4	// clock, timer, alarm, stopwatch
#define NUM_CLOCKFIELDS 	5	// min, hr, year, month, day (year and month first for getting bounds of day)
#define NUM_TIMERFIELDS 	3	// sec, min, hr
#define NUM_ALARMFIELDS 	4	// sec, min, hr, day of week
#define NUM_STOPWATCHFIELDS 4	// ms, sec, min, hr

// structs to hold related flags/variables together.
struct faceFlags {
	uint8_t clock;
	uint8_t timer;
	uint8_t alarm;
	uint8_t stopwatch;
};

struct clockVariables {
	uint8_t isBeingSet;
	uint8_t fieldBeingSet;
	struct dates *dateToSet;
	struct times *timeToSet;
};

struct timerVariables {
	uint8_t isBeingSet;
	uint8_t fieldBeingSet;
	uint8_t isSet;
	struct times *timeToSet;
};

struct alarmVariables {
	uint8_t isBeingSet;
	uint8_t fieldBeingSet;
	uint8_t isSet;
	struct alarmTimes *alarmToSet;
};

struct stopwatchVariables {
	uint32_t lapPrev;
	uint32_t lapCurrent;
};

struct buttonFlags {
	uint8_t is1Pressed;
	uint8_t is2Pressed;
	uint8_t is3Pressed;
	uint8_t is4Pressed;
};

// volatile and global since other files need to use/see in their callbacks
volatile struct buttonFlags buttons;	// updated when button is pressed
volatile struct faceFlags updateFace;	// updated when display has to be changed somehow
volatile uint8_t isTimerRunning;		// tracks when timer is running (already set, not paused)
volatile uint8_t isStopwatchRunning;	// tracks when stopwatch is running (not paused, not cleared)
volatile uint8_t isTimerPaused;			// used by timers.c file to see if function was paused
volatile uint8_t isStopwatchPaused;		// tracks when stopwatch is paused (not cleared)
volatile uint8_t isTimerDone;			// tracks when timer has run to completion (not paused)
volatile uint8_t isAlarmDone;			// tracks when alarm has run to completion

// enum for different faces used (clock, timer, alarm, stopwatch)
enum displayFaces {
	faceClock,
	faceTimer,
	faceAlarm,
	faceStopwatch
};

// months and weekdays often represented as ints. used to translate from these ints to string
// size is because of 1-based indexing that HAL uses
const char* weekdayNames[8];
const char* monthNames[13];

// ---- main functions ----
// holds state of the system based on button presses
void updateState(RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *motorBacklightTim, TIM_HandleTypeDef *buttonTim, SPI_HandleTypeDef *hspi);

// primary function for updating display according to different factors
void updateDisplay(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
// ---- end of main functions ----

// ---- helper functions ----
// helper state functions. only called when their respective program is on the screen
void updateClockState(RTC_HandleTypeDef *hrtc);
void updateTimerState(TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *motorTim);
void updateAlarmState(RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *motorTim);
void updateStopwatchState(TIM_HandleTypeDef *timerStopwatchTim);

// helper display functions. only called when their respective program is on the screen.
void updateClockDisplay(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void updateTimerDisplay(SPI_HandleTypeDef *hspi);
void updateAlarmDisplay(SPI_HandleTypeDef *hspi);
void updateStopwatchDisplay(SPI_HandleTypeDef *hspi);
// ---- end of helper functions

// ---- graphics functions ----
// draws graphics that represent buttons and what the buttons are supposed to do
void drawButton(uint8_t x_center, uint8_t y_center, SPI_HandleTypeDef* hspi);
void drawButtons(SPI_HandleTypeDef *hspi);
void drawButtonText(const char *str1, const char *str2, const char *str3, SPI_HandleTypeDef *hspi);

// draws a big title at the top that says what's displayed on the screeen
void drawTitle(char *str, SPI_HandleTypeDef *hspi);

// draws a graphic for the current battery level
void drawBattery(uint8_t batteryLevel, SPI_HandleTypeDef *hspi);

// draws the primary clock display. hr:min:sec, mm/dd/yyyy, weekday
void drawClock(struct dates *d, struct times *t, SPI_HandleTypeDef *hspi);

// draws current time on top of screen
void drawTopClock(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);

// draws the primary timer display. hr:min:sec
void drawTimer(struct times *t, SPI_HandleTypeDef *hspi);

// draws the main alarm display. hr:min:sec weekday
void drawAlarm(struct alarmTimes *a, SPI_HandleTypeDef *hspi);

// draws the main stopwatch display. hr:min:sec
void drawStopwatch(uint32_t seconds, SPI_HandleTypeDef *hspi);
// draws the lap value. hr:min:sec
void drawStopwatchLap(uint32_t seconds, SPI_HandleTypeDef *hspi);
// ---- end of graphics functions

// initialization function. initializes variables at the start of execution
void initFace();

#endif
