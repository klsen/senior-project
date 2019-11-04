///*
// * file for initializing and working with the real-time clock
// * and low-power/general purpose timers for time-keeping functionality
// *
// * called in main(). uses global variables?
// */
//
//// includes
//#include "stm32l4xx_hal.h";
//
//// ---- functions used ---- (move to header file)
//// set_clock();
//// set_timer();
//// set_alarm();
//
//// irq handlers?
////
//
//void setTime(RTC_HandleTypeDef *hrtc) {
////	RTC_TimeTypeDef stime = {
////		Hours = 0,
////		Minutes = 0,
////		Seconds = 0,
////	};
//
//	RTC_TimeTypeDef stime = {0};	// change to malloc call? does that work in embedded?
//
//	// set using args later
//	stime.Hours = 0;
//	stime.Minutes = 0;
//	stime.Seconds = 0;
//
//	stime.TimeFormat = RTC_HOURFORMAT_24;
//
//	// not sure what these do
//	stime.SubSeconds = 0;
//	stime.SecondFraction = 0;
//
//	stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;		// add daylight savings later?
//	stime.StoreOperation = RTC_STOREOPERATION_SET;		// not sure what this does
//
//	// do nothing until done
//	// bcd format vs binary format: what does the library natively use?
//	while (HAL_RTC_SetTime(hrtc, &stime, RTC_FORMAT_BCD) != HAL_OK);
//}
//
//void setDateTime(RTC_HandleTypeDef *hrtc) {
//	// ---- time ----
//	RTC_TimeTypeDef stime = {0};
//
//	// set with args later
//	stime.Hours = 0;
//	stime.Minutes = 0;
//	stime.Seconds = 0;
//
//	stime.TimeFormat = RTC_HOURFORMAT_24;
//
//	// not sure what these do
//	stime.SubSeconds = 0;
//	stime.SecondFraction = 0;
//
//	stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;		// add daylight savings later?
//	stime.StoreOperation = RTC_STOREOPERATION_SET;		// not sure what this does
//	// ---- end time ----
//
//	// ---- date ----
//	RTC_DateTypeDef sdate = {0};
//
//	// set with args later
//	sdate.WeekDay = RTC_WEEKDAY_MONDAY;
//	sdate.Month = RTC_MONTH_JANUARY;
//	sdate.Date = 1;
//	sdate.Year = 19; 		// set only between 0-99. part of the library (!?)
//	// ---- end date ----
//
//	// do nothing until done
//	while (HAL_RTC_SetTime(hrtc, &stime, RTC_FORMAT_BCD) != HAL_OK);
//	while (HAL_RTC_SetDate(hrtc, &sdate, RTC_FORMAT_BCD) != HAL_OK);
//}
//
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
//	salarm.Alarm = RTC_ALARM_A;		// define for which alarm hardware to use
//
//	// do nothing until done
//	while (HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BCD) != HAL_OK);
//}
//
//// for time of day+week
//void setAlarm(RTC_HandleTypeDef *hrtc) {
//	RTC_AlarmTypeDef salarm = {0};
//	RTC_TimeTypeDef salarmtime = {0};
//
//	// change to set with args
//	salarmtime.Hours = 0;
//	salarmtime.Minutes = 0;
//	salarmtime.Seconds = 0;
//	salarmtime.TimeFormat = RTC_HOURFORMAT_24;
//	salarmtime.SubSeconds = 0;
//	salarmtime.SecondFraction = 0;
//	salarmtime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//	salarmtime.StoreOperation = RTC_STOREOPERATION_RESET;
//
//	salarm.AlarmTime = salarmtime;
//	salarm.AlarmMask = RTC_ALARMMASK_ALL;
//	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
//	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
//	salarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
//	salarm.Alarm = RTC_ALARM_B;
//
//	// do nothing until done
//	while (HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BCD) != HAL_OK);
//}
//
//// thinking we have to implement alarm callbacks instead of irq handler
//// callbacks defined as "weak" in library, irq handler is not
//// who knows if this is even gonna work
//// how to write callback: we're only given rtc handle
////   check for flags, run based on flags, clear flags(?)
////   check state typedef (nvm, only has ready, reset, busy, timeout, error)
////   lock object? (enum that only has locked and unlocked status)
//
//// rtc handle: contains rtc base address, init, lock, state
////   none look useful >:(
////   gonna use peripheral addresses?
////   getState()? (getState is useless, only returns state typedef ^)
//
//// check rtc exported macros section
//// get/clear flag macro (difference between exti flag macros?)
////   bunch of flag definitions for different flags :O
//void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {}
//
//void HAL_RTC_AlarmBEventCallback(RTC_HandleTypeDef *hrtc) {}
