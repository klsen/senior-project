// header file for using RTC

#ifndef CLOCKS_H
#define CLOCKS_H

#include "stm32l0xx_hal.h"
#include "TFT_display.h"
#include <stdio.h>			// for sprintf

/*
 * l0 pinout:
 * spi: pa5 = sck, pa7 = mosi
 * buttons: pb2, pb13, pb14, pb15
 *
 * RTC CALIBRATION FOR CLOCK CRYSTAL ON RTC EXTENSIONS FILE
 */

// ---- variables ----
struct alarmTimes watchAlarm;		// for keeping track of what alarms are in the RTC.
struct alarmTimes watchTimer;		// sharing 1 alarm hardware between 2 times (is this necessary?)
uint32_t watchTimerSeconds;
// ---- end of variables ----

// ---- personal structs ----
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
// ---- end of structs ----

// change to follow convention set by TFT_display? other params, then peripheral handle?
// ---- setting rtc time ----
void setTime(RTC_HandleTypeDef *hrtc, struct times *t);
void setDate(RTC_HandleTypeDef *hrtc, struct dates *d);
void setDateTime(RTC_HandleTypeDef *hrtc, struct dates *d, struct times *t);
// ---- end of rtc setters ----

// ---- alarm setters ----
void setAlarm(RTC_HandleTypeDef *hrtc, struct alarmTimes *a);
void setAlarmB(RTC_HandleTypeDef *hrtc);
void setTimer(RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *htim, struct times *t_in);
// ---- end of alarm setters ----

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
