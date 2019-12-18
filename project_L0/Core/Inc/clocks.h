/*
 * Header file for using built-in RTC hardware on ST microprocessors.
 *
 * Uses RTC's calendar, alarm, and calibration features.
 * Includes converters and calculators.
 */

#ifndef CLOCKS_H
#define CLOCKS_H

#include "stm32l0xx_hal.h"
#include "user_interface.h"
#include "battery.h"

// ---- structs ----
// used to hold related variables for time, date, and alarm
struct times {
	uint8_t hr;
	uint8_t min;
	uint8_t sec;
};

struct dates {
	uint16_t yr;
	uint8_t month;
	uint8_t date;
	uint8_t weekday;		// in hal, week starts monday counting from 1
};

struct alarmTimes {
	uint8_t hr;
	uint8_t min;
	uint8_t sec;
	uint8_t weekday;		// special because it's time, but needs weekday
};
// ---- end of structs ----

// ---- RTC setters ----
void setTime(struct times *t, RTC_HandleTypeDef *hrtc);
void setDate(struct dates *d, RTC_HandleTypeDef *hrtc);
void setDateTime(struct dates *d, struct times *t, RTC_HandleTypeDef *hrtc);
// ---- end of RTC setters ----

// ---- alarm setters ----
void setAlarm(struct alarmTimes *a, RTC_HandleTypeDef *hrtc);
void setClockAlarm(RTC_HandleTypeDef *hrtc);		// set alarm to current time+1 to trigger display update every 1s
// ---- end of alarm setters ----

// ---- RTC getters ----
void getTime(struct times *t, RTC_HandleTypeDef *hrtc);
void getDate(struct dates *d, RTC_HandleTypeDef *hrtc);
void getDateTime(struct dates *d, struct times *t, RTC_HandleTypeDef *hrtc);
// ---- end of RTC getters ----

// rtc calibration function. adjusts LSE input that goes into RTC to accomodate for LSE ppm
void setRTCCalibration(int calibVal, RTC_HandleTypeDef *hrtc);

// ---- converters and calculators ----
// converts time struct to seconds and vice-versa
uint32_t timeToSeconds(struct times *t);
void secondsToTime(struct times *t, uint32_t seconds);

// calculates weekday according to a formula. pulled from Wikipedia.
// should be accurate for all dates of the Gregorian calendar
uint8_t weekdayCalculator(uint16_t year, uint8_t month, uint8_t day);

// returns max days in a month (jan=31, feb=28, ...)
uint8_t maxDaysInMonth(uint8_t month, uint16_t year);
// ---- end of converters and calculators ----

#endif
