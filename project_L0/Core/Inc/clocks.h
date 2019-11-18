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
uint32_t alarmMask;
struct times watchTimer;			// using RTC alarmB for this now
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
void setTime(struct times *t, RTC_HandleTypeDef *hrtc);
void setDate(struct dates *d, RTC_HandleTypeDef *hrtc);
void setDateTime(struct dates *d, struct times *t, RTC_HandleTypeDef *hrtc);
// ---- end of rtc setters ----

// ---- alarm setters ----
void setAlarm(struct alarmTimes *a, RTC_HandleTypeDef *hrtc);
void setAlarmB(RTC_HandleTypeDef *hrtc);
void setTimer(struct times *t_in, RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *htim);
// ---- end of alarm setters ----

// ---- getting rtc times ----
void getTime(struct times *t, RTC_HandleTypeDef *hrtc);
void getDate(struct dates *d, RTC_HandleTypeDef *hrtc);
void getDateTime(struct dates *d, struct times *t, RTC_HandleTypeDef *hrtc);
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
