/*
 * file for initializing and working with the real-time clock
 * and low-power/general purpose timers for time-keeping functionality
 *
 * called in main(). uses global variables?
 */


#include "clocks.h"

// set rtc time. uses perosnal struct as arg
void setTime(RTC_HandleTypeDef *hrtc, struct times *t) {
	RTC_TimeTypeDef stime = {0};	// change to malloc call? does that work in embedded?

	// set using args later
	stime.Hours = t->hr;
	stime.Minutes = t->min;
	stime.Seconds = t->sec;

	stime.TimeFormat = RTC_HOURFORMAT_24;

	// not sure what these do, but probably fine if set to 0 or ignored
	stime.SubSeconds = 0;
	stime.SecondFraction = 0;

	stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;		// add daylight savings later?
	stime.StoreOperation = RTC_STOREOPERATION_SET;		// not sure what this does

	// do nothing until done
	// not following BCD format (4-bit digit 1, 4-bit digit 2)
	// while makes program hang? ignore instead?
//	while (HAL_RTC_SetTime(hrtc, &stime, RTC_FORMAT_BIN) != HAL_OK);
	HAL_RTC_SetTime(hrtc, &stime, RTC_FORMAT_BIN);
}

// set rtc date. uses personal struct
void setDate(RTC_HandleTypeDef *hrtc, struct dates *d) {
	// ---- date ----
	RTC_DateTypeDef sdate = {0};

	sdate.Month = d->month;
	sdate.Date = d->date;
	sdate.WeekDay = d->weekday;
	sdate.Year = d->yr % 100; 		// set only between 0-99. part of the library (!?)

	HAL_RTC_SetDate(hrtc, &sdate, RTC_FORMAT_BIN);
}

void setDateTime(RTC_HandleTypeDef *hrtc, struct dates *d, struct times *t) {
	setDate(hrtc, d);
	setTime(hrtc, t);
}

// for time of day+week
void setAlarm(RTC_HandleTypeDef *hrtc, struct alarmTimes *a) {
	RTC_AlarmTypeDef salarm = {0};
	RTC_TimeTypeDef salarmtime = {0};

	// change to set with args
	salarmtime.Hours = a->hr;
	salarmtime.Minutes = a->min;
	salarmtime.Seconds = a->sec;
	salarmtime.TimeFormat = RTC_HOURFORMAT_24;
	salarmtime.SubSeconds = 0;
	salarmtime.SecondFraction = 0;
	salarmtime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	salarmtime.StoreOperation = RTC_STOREOPERATION_RESET;

	salarm.AlarmTime = salarmtime;
	salarm.AlarmMask = RTC_ALARMMASK_ALL;
	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	salarm.AlarmDateWeekDay = a->weekday;
	salarm.Alarm = RTC_ALARM_A;			// change if using different alarm

	// do nothing until done
	while (HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BCD) != HAL_OK);
}

//// for no. of seconds
//// using RTC alarm A
//// thinking of using timer instead of rtc alarm for this
////   interrupts on a second granularity for display? not sure of rtc has
//void setTimer(RTC_HandleTypeDef *hrtc) {
//	RTC_AlarmTypeDef salarm = {0};
//
//	RTC_TimeTypeDef salarmtime = {0};
//	// fields we actually care about
//	salarmtime.Hours = 0;
//	salarmtime.Minutes = 0;
//	salarmtime.Seconds = 0;
//	salarmtime.TimeFormat = RTC_HOURFORMAT_24;
//	// idc about the rest of these?
//	salarmtime.SubSeconds = 0;
//	salarmtime.SecondFraction = 0;
//	salarmtime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//	salarmtime.StoreOperation = RTC_STOREOPERATION_RESET; // not even sure if we need this
//
//	// change to set with args
//	salarm.AlarmTime = salarmtime;		// uses time struct
//	salarm.AlarmMask = RTC_ALARMMASK_ALL;		// not sure what this does. research more
//	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;		// no sub-second comparison. research what this does more
//	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
//	salarm.AlarmDateWeekDay = 1;	// change to calculate using current clock date
//	salarm.Alarm = RTC_ALARM_B;		// define for which alarm hardware to use
//
//	// do nothing until done
//	while (HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BIN) != HAL_OK);
//}

// thinking we have to implement alarm callbacks instead of irq handler
// callbacks defined as "weak" in library, irq handler is not
// who knows if this is even gonna work
// how to write callback: we're only given rtc handle
//   check for flags, run based on flags, clear flags(?)
//   check state typedef (nvm, only has ready, reset, busy, timeout, error)
//   lock object? (enum that only has locked and unlocked status)

// rtc handle: contains rtc base address, init, lock, state
//   none look useful >:(
//   gonna use peripheral addresses?
//   getState()? (getState is useless, only returns state typedef ^)

// check rtc exported macros section
// get/clear flag macro (difference between exti flag macros?)
//   bunch of flag definitions for different flags :O
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
	// change pin to whatever's accessible
	// using PC0
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
}

// ---- clock get functions ----

// maybe needs subseconds?
void getTime(RTC_HandleTypeDef *hrtc, struct times *t) {
	RTC_TimeTypeDef stime;

	// programming manual says to read time after date. something shadow registers.
	// not done automatically in HAL
	HAL_RTC_GetTime(hrtc, &stime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(hrtc, NULL, RTC_FORMAT_BIN);

	t->hr = stime.Hours;
	t->min = stime.Minutes;
	t->sec = stime.Seconds;
}

void getDate(RTC_HandleTypeDef *hrtc, struct dates *d) {
	RTC_DateTypeDef sdate;

	// programming manual says to read time after date. something shadow registers.
	// not done automatically in HAL
	HAL_RTC_GetTime(hrtc, NULL, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(hrtc, &sdate, RTC_FORMAT_BIN);

	d->yr = sdate.Year > 50 ? sdate.Year+1900 : sdate.Year+2000;
	d->month = sdate.Month;
	d->date = sdate.Date;
	d->weekday = sdate.WeekDay;
}

// not using getDate and getTime for efficiency (?)
void getDateTime(RTC_HandleTypeDef *hrtc, struct dates *d, struct times *t) {
	RTC_DateTypeDef sdate;
	RTC_TimeTypeDef stime;

	// programming manual says to read time after date. something shadow registers.
	// not done automatically in HAL
	HAL_RTC_GetTime(hrtc, &stime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(hrtc, &sdate, RTC_FORMAT_BIN);

	d->yr = sdate.Year > 50 ? sdate.Year+1900 : sdate.Year+2000;
	d->month = sdate.Month;
	d->date = sdate.Date;
	d->weekday = sdate.WeekDay;

	t->hr = stime.Hours;
	t->min = stime.Minutes;
	t->sec = stime.Seconds;
}
// ---- end of clock get functions ----

// ---- clock print functions ----
// print functions for RTC
// assumes we're using SPI display and file TFT_display.c
// pulls date and time structs automatically to only print current time in RTC
void printTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	char str[40];		// problems when using only char*

	struct times t;
	getTime(hrtc, &t);

	sprintf(str, "sec: %2d", t.sec);
	drawText(0, 0, 1, ST77XX_BLACK, ST77XX_WHITE, str, hspi);
	sprintf(str, "min: %2d", t.min);
	drawText(0, 10, 1, ST77XX_BLACK, ST77XX_WHITE, str, hspi);
	sprintf(str, "hr: %3d", t.hr);
	drawText(0, 20, 1, ST77XX_BLACK, ST77XX_WHITE, str, hspi);
}

void printDate(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	char str[40];		// problems when using only char*

	struct dates d;
	getDate(hrtc, &d);

	sprintf(str, "year: %3d", d.yr);
	drawText(0, 40, 1, ST77XX_BLACK, ST77XX_WHITE, str, hspi);
	sprintf(str, "month: %4d", d.month);
	drawText(0, 50, 1, ST77XX_BLACK, ST77XX_WHITE, str, hspi);
	sprintf(str, "day: %2d", d.date);
	drawText(0, 60, 1, ST77XX_BLACK, ST77XX_WHITE, str, hspi);
	sprintf(str, "day: %2d", d.weekday);	// probably not gonna show nice since its an enum
	drawText(0, 70, 1, ST77XX_BLACK, ST77XX_WHITE, str, hspi);
}

void printDateTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	printTime(hrtc, hspi);
	printDate(hrtc, hspi);
}
// ---- end of clock print functions ----

// tests clock functions. assumes SPI display using TFT_display.c is available
void clockTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	struct times t = {1, 1, 1};
	struct dates d = {19, 11, 13};

	HAL_Delay(1000);
	printDateTime(hrtc, hspi);
	HAL_Delay(2000);
	printDateTime(hrtc, hspi);

	HAL_Delay(1000);
	setTime(hrtc, &t);
	printDateTime(hrtc, hspi);
	HAL_Delay(1000);
	printDateTime(hrtc, hspi);

	HAL_Delay(1000);
	setDateTime(hrtc, &d, &t);
	printDateTime(hrtc, hspi);
	HAL_Delay(1000);
	printDateTime(hrtc, hspi);
}

void alarmTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	struct dates d;
	struct times t;

	getDateTime(hrtc, &d, &t);

	if (t.sec > 60) t.min += 1;
	t.sec = (t.sec+10) % 60;
	struct alarmTimes a = {t.hr, t.min, t.sec, d.weekday};

	setAlarm(hrtc, &a);
}
