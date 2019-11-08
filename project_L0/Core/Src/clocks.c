/*
 * file for initializing and working with the real-time clock
 * and low-power/general purpose timers for time-keeping functionality
 *
 * called in main(). uses global variables?
 */


#include "clocks.h"

// ---- functions used ---- (move to header file)
// set_clock();
// set_timer();
// set_alarm();


// change to use arguments
void setTime(RTC_HandleTypeDef *hrtc) {
	RTC_TimeTypeDef stime = {0};	// change to malloc call? does that work in embedded?

	// set using args later
	stime.Hours = 1;
	stime.Minutes = 1;
	stime.Seconds = 1;

	stime.TimeFormat = RTC_HOURFORMAT_24;

	// not sure what these do, but probably fine if set to 0 or ignored
	stime.SubSeconds = 0;
	stime.SecondFraction = 0;

	stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;		// add daylight savings later?
	stime.StoreOperation = RTC_STOREOPERATION_SET;		// not sure what this does

	// do nothing until done
	// not following BCD format (4-bit digit 1, 4-bit digit 2)
	// while makes program hang? ignore instead?
	while (HAL_RTC_SetTime(hrtc, &stime, RTC_FORMAT_BIN) != HAL_OK);
}

// change to use args
void setDateTime(RTC_HandleTypeDef *hrtc) {
	// ---- time ----
	RTC_TimeTypeDef stime = {0};

	// set with args later
	stime.Hours = 2;
	stime.Minutes = 2;
	stime.Seconds = 2;

	stime.TimeFormat = RTC_HOURFORMAT_24;

	// not sure what these do
	stime.SubSeconds = 0;
	stime.SecondFraction = 0;

	stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;		// add daylight savings later?
	stime.StoreOperation = RTC_STOREOPERATION_SET;		// not sure what this does
	// ---- end time ----

	// ---- date ----
	RTC_DateTypeDef sdate = {0};

	// set with args later
	sdate.Month = RTC_MONTH_NOVEMBER;
	sdate.Date = 8;
	sdate.WeekDay = RTC_WEEKDAY_FRIDAY;
	sdate.Year = 19; 		// set only between 0-99. part of the library (!?)
	// ---- end date ----

	// do nothing until done
	while (HAL_RTC_SetTime(hrtc, &stime, RTC_FORMAT_BIN) != HAL_OK);
	while (HAL_RTC_SetDate(hrtc, &sdate, RTC_FORMAT_BIN) != HAL_OK);
}

// for time of day+week
void setAlarm(RTC_HandleTypeDef *hrtc) {
	RTC_AlarmTypeDef salarm = {0};
	RTC_TimeTypeDef salarmtime = {0};

	// change to set with args
	salarmtime.Hours = 0;
	salarmtime.Minutes = 0;
	salarmtime.Seconds = 0;
	salarmtime.TimeFormat = RTC_HOURFORMAT_24;
	salarmtime.SubSeconds = 0;
	salarmtime.SecondFraction = 0;
	salarmtime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	salarmtime.StoreOperation = RTC_STOREOPERATION_RESET;

	salarm.AlarmTime = salarmtime;
	salarm.AlarmMask = RTC_ALARMMASK_ALL;
	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	salarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
	salarm.Alarm = RTC_ALARM_B;

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
//void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {}

//void HAL_RTC_AlarmBEventCallback(RTC_HandleTypeDef *hrtc) {}

// ---- clock print functions ----
// print functions for RTC
// assumes we're using SPI display and file TFT_display.c
// pulls date and time structs automatically to only print current time in RTC
char* printTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	char* str;

	RTC_TimeTypeDef *stime;
	HAL_RTC_getTime(hrtc, stime, RTC_FORMAT_BIN);

	uint8_t sec = stime->Seconds;
	uint8_t min = stime->Minutes;
	uint8_t hr = stime->Hours;
	sprintf(str, "sec: %2d, min: %2d, hour: %2d", sec, min, hr);
	drawText(0, 0, 1, ST77XX_BLACK, ST77XX_WHITE, str, hspi);
}

char* printDate(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	char* str;

	RTC_DateTypeDef *sdate;
	HAL_RTC_getDate(hrtc, stime, RTC_FORMAT_BIN);

	uint8_t yr = sdate->Year;
	uint8_t month = sdate->Month;
	uint8_t day = sdate->Date;
	sprintf(str, "year: %2d, month: %2d, day: %2d");
	drawText(0, 10, 1, ST77XX_BLACK, ST77XX_WHITE, str, hspi);
}

char* printDateTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	printDate(hrtc, hspi);
	printTime(hrtc, hspi);
}
// ---- end of clock print functions ----

// tests clock functions. assumes SPI display using TFT_display.c is available
void clockTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	HAL_Delay(1000);
	printDateTime(hrtc, hspi);
	HAL_Delay(2000);
	printDateTime(hrtc, hspi);

	HAL_Delay(1000);
	setTime(hrtc);
	printDateTime(hrtc, hspi);
	HAL_Delay(1000);
	printDateTime(hrtc, hspi);

	HAL_Delay(1000);
	setDateTime(hrtc);
	printDateTime(hrtc, hspi);
	HAL_Delay(1000);
	printDateTime(hrtc, hspi);
}
