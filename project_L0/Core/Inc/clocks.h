// header file for using RTC and timers.

#ifndef CLOCKS_H
#define CLOCKS_H

#include "stm32l0xx_hal.h"
#include "TFT_display.h"
#include <stdio.h>

/*
 * l0 pinout:
 * spi: pa5 = sck, pa7 = mosi
 * buttons: pb2, pb13, pb14, pb15
 */

struct times {
	uint8_t hr;
	uint8_t min;
	uint8_t sec;
};

// weekday can be omitted? 0 if not set
struct dates {
	uint8_t yr;
	uint8_t month;
	uint8_t date;
	uint8_t weekday;		// in hal, week starts monday counting from 1
};

// struct for alarm stuff
struct alarmTimes {
	uint8_t hr;
	uint8_t min;
	uint8_t sec;
	uint8_t weekday;
};

// ---- setting rtc time ----
void setTime(RTC_HandleTypeDef *hrtc, struct times *t);
void setDate(RTC_HandleTypeDef *hrtc, struct dates *d);
void setDateTime(RTC_HandleTypeDef *hrtc, struct dates *d, struct times *t);
// ---- end of rtc setters ----

// setting alarm
void setAlarm(RTC_HandleTypeDef *hrtc, struct alarmTimes *a);
void setAlarmB(RTC_HandleTypeDef *hrtc);

// ---- getting rtc times ----
void getTime(RTC_HandleTypeDef *hrtc, struct times *t);
void getDate(RTC_HandleTypeDef *hrtc, struct dates *d);
void getDateTime(RTC_HandleTypeDef *hrtc, struct dates *d, struct times *t);
// ---- end of getters ----

// ---- printing stuff ----
void printTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void printDate(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void printDateTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
// ---- end of prints ----

// ---- tests ----
void clockTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void alarmTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
// ---- end of tests ----

#endif
