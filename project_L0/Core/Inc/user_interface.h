/*
 * file for handling navigation between different apps/interfaces.
 * uses global flags (concurrency problem?). Displays information
 * using functions defined in TFT_display.c.
 */

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "stm32l0xx_hal.h"
#include "TFT_display.h"
#include "clocks.h"
#include "timers.h"
#include "main.h"
#include <stdlib.h>		// for malloc

// uses gpio external interrupts. interrupt handler/callback function
// ignores ports, so only pin number is needed.
#define BUTTON1 GPIO_PIN_2
#define BUTTON2 GPIO_PIN_13
#define BUTTON3 GPIO_PIN_14
#define BUTTON4 GPIO_PIN_15

// defines to help with flags
#define NUM_FACES 4
#define NUM_CLOCKFIELDS 5		// min, hr, year, month, day (year, month first for getting bounds of day)
#define NUM_TIMERFIELDS 3		// sec, min, hr
#define NUM_ALARMFIELDS 4		// sec, min, hr, day of week
#define NUM_STOPWATCHFIELDS 4	// ms, sec, min, hr

// variables to track state of apps, set in callback/interrupt
// straight global instead of static/scope in file only?
// need volatile keyword?

// structs to hold related flags/variables together.
// maybe change to not use these? they're starting to get really annoying to use. or keep for the organization?
struct faceFlags {
//	uint8_t face;			// change to static, others global
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
//	uint8_t isRunning;			// global
	struct times *timeToSet;
};

struct alarmVariables {
	uint8_t isBeingSet;
	uint8_t fieldBeingSet;
	uint8_t isSet;
//	uint8_t isRunning;				// global
	struct alarmTimes *alarmToSet;
};

struct stopwatchVariables {
//	uint8_t isRunning;			// global
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
volatile struct buttonFlags buttons;
volatile struct faceFlags updateFace;
volatile uint8_t isTimerRunning;
volatile uint8_t isStopwatchRunning;
volatile uint8_t isTimerPaused;			// used by timers.c file to see if function was paused
volatile uint8_t isStopwatchPaused;
volatile uint8_t isTimerDone;
volatile uint8_t isAlarmDone;

//volatile uint8_t buttonPressed;

// enum for different faces used (clock, timer, alarm, stopwatch)
// probably not needed
enum displayFaces {
	faceClock,
	faceTimer,
	faceAlarm,
	faceStopwatch
};

const char* weekdayNames[8];
const char* monthNames[13];

// might need hw timer and rtc handles but not if they're kept global.
// refactor clock and timer back to use non-global
void updateState(RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *motorBacklightTim, TIM_HandleTypeDef *buttonTim);
void updateDisplay(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);

// getting really sick of looking at 1 function with 250 lines.
void updateClockState(RTC_HandleTypeDef *hrtc);
void updateTimerState(TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *motorTim);
void updateAlarmState(RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *motorTim);
void updateStopwatchState(TIM_HandleTypeDef *timerStopwatchTim);

// getting really sick of looking at 1 function with 150 lines
void updateClockDisplay(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void updateTimerDisplay(SPI_HandleTypeDef *hspi);
void updateAlarmDisplay(SPI_HandleTypeDef *hspi);
void updateStopwatchDisplay(SPI_HandleTypeDef *hspi);

// draw functions
void drawButton(uint8_t x_center, uint8_t y_center, SPI_HandleTypeDef* hspi);
void drawButtons(SPI_HandleTypeDef *hspi);
void drawButtonText(const char *str1, const char *str2, const char *str3, SPI_HandleTypeDef *hspi);
void drawTitle(char *str, SPI_HandleTypeDef *hspi);
void drawClock(struct dates *d, struct times *t, SPI_HandleTypeDef *hspi);
void drawTimer(struct times *t, SPI_HandleTypeDef *hspi);
void drawAlarm(struct alarmTimes *a, SPI_HandleTypeDef *hspi);
void drawStopwatch(uint32_t seconds, SPI_HandleTypeDef *hspi);
void drawStopwatchLap(uint32_t seconds, SPI_HandleTypeDef *hspi);

uint8_t maxDaysInMonth(uint8_t month, uint16_t year);
void initFace();

#endif
