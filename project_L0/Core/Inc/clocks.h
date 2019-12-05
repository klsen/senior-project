// header file for using RTC

#ifndef CLOCKS_H
#define CLOCKS_H

#include "stm32l0xx_hal.h"
#include "TFT_display.h"
#include "timers.h"
#include "main.h"
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
struct times watchTimer;			// using RTC alarmB for this now
volatile uint32_t watchTimerSeconds;
// ---- end of variables ----

// ---- personal structs ----
struct times {
	uint8_t hr;
	uint8_t min;
	uint8_t sec;
};

// weekday can be omitted? 0 if not set
struct dates {
	uint16_t yr;
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

// ---- setting rtc time ----
//void setTime(struct times *t, RTC_HandleTypeDef *hrtc);
//void setDate(struct dates *d, RTC_HandleTypeDef *hrtc);
//void setDateTime(struct dates *d, struct times *t, RTC_HandleTypeDef *hrtc);
void setTime(struct times *t);
void setDate(struct dates *d);
void setDateTime(struct dates *d, struct times *t);
// ---- end of rtc setters ----

// ---- alarm setters ----
//void setAlarm(struct alarmTimes *a, RTC_HandleTypeDef *hrtc);
//void setTimer(struct times *t_in, RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *htim);
void setAlarm(struct alarmTimes *a);
void setTimer(struct times *t_in);
// ---- end of alarm setters ----

// ---- getting rtc times ----
//void getTime(struct times *t, RTC_HandleTypeDef *hrtc);
//void getDate(struct dates *d, RTC_HandleTypeDef *hrtc);
//void getDateTime(struct dates *d, struct times *t, RTC_HandleTypeDef *hrtc);
void getTime(struct times *t);
void getDate(struct dates *d);
void getDateTime(struct dates *d, struct times *t);
// ---- end of getters ----

// converters
uint32_t timeToSeconds(struct times *t);
void secondsToTime(struct times *t, uint32_t seconds);

// ---- printing stuff ----
//void printTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
//void printDate(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
//void printDateTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void printTime();
void printDate();
void printDateTime();
// ---- end of prints ----

// ---- tests ----
//void clockTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
//void alarmTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void clockTest();
void alarmTest();
void timerTest();
// ---- end of tests ----

#endif
