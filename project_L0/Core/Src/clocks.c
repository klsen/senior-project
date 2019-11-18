/*
 * file for initializing and working with the real-time clock
 * and low-power/general purpose timers for time-keeping functionality
 *
 * called in main(). uses global variables?
 */


#include "clocks.h"
#include "timers.h"

// set rtc time. uses perosnal struct as arg
void setTime(struct times *t, RTC_HandleTypeDef *hrtc) {
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
void setDate(struct dates *d, RTC_HandleTypeDef *hrtc) {
	// ---- date ----
	RTC_DateTypeDef sdate = {0};

	sdate.Month = d->month;
	sdate.Date = d->date;
	sdate.WeekDay = d->weekday;
	sdate.Year = d->yr % 100; 		// set only between 0-99. part of the library (!?)

	HAL_RTC_SetDate(hrtc, &sdate, RTC_FORMAT_BIN);
}

void setDateTime(struct dates *d, struct times *t, RTC_HandleTypeDef *hrtc) {
	setDate(d, hrtc);
	setTime(t, hrtc);
}

// for time of day+week
void setAlarm(struct alarmTimes *a, RTC_HandleTypeDef *hrtc) {
	RTC_AlarmTypeDef salarm = {0};
	RTC_TimeTypeDef salarmtime = {0};

	watchAlarm = *a;	// this is probably fine (value at a is defined already)

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
	salarm.AlarmMask = RTC_ALARMMASK_NONE;
	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	salarm.AlarmDateWeekDay = a->weekday;
	salarm.Alarm = RTC_ALARM_A;			// change if using different alarm

	// do nothing until done
//	while (HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BCD) != HAL_OK);
	HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BIN);
}

// set an alarm 1 sec after current time to produce signal every sec
// i hope this doesnt take too much time (haha)
// should probably use this 2nd alarm for timer function instead of wasting it like this lol
// can use one of the hw timers to do this instead?
void setAlarmB(RTC_HandleTypeDef *hrtc) {
	RTC_AlarmTypeDef salarm = {0};
	RTC_TimeTypeDef salarmtime = {0};

	struct dates d;
	struct times t;
	getDateTime(&d, &t, hrtc);

	// if tree to increment time by 1 sec
	if (t.sec + 1 > 60) {
		if (t.min + 1 > 60) {
			if (t.hr + 1 > 24) {
				d.weekday = ((d.weekday+1) % 7) + 1;
			}
			t.hr = (t.hr + 1) % 24;
		}
		t.min = (t.min + 1) % 60;
	}
	t.sec = (t.sec + 1) % 60;

	// change to set with args
	salarmtime.Hours = t.hr;
	salarmtime.Minutes = t.min;
	salarmtime.Seconds = t.sec;
	salarmtime.TimeFormat = RTC_HOURFORMAT_24;
	salarmtime.SubSeconds = 0;
	salarmtime.SecondFraction = 0;
	salarmtime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	salarmtime.StoreOperation = RTC_STOREOPERATION_RESET;

	salarm.AlarmTime = salarmtime;
	salarm.AlarmMask = RTC_ALARMMASK_ALL;
	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	salarm.AlarmDateWeekDay = d.weekday;
	salarm.Alarm = RTC_ALARM_B;			// change if using different alarm

	HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BIN);
}

// set alarm for timer function of watch project
// using RTC alarm hardware
void setTimer(struct times *t_in, RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *htim) {
	RTC_AlarmTypeDef salarm = {0};
	RTC_TimeTypeDef salarmtime = {0};

	// set global variables to hold value being set
	watchTimer = *t_in;
	watchTimerSeconds = t_in->sec + t_in->min*60 + t_in->hr * 3600;

	// pull current RTC time
	struct dates d;
	struct times t;
	getDateTime(&d, &t, hrtc);

	struct alarmTimes a;

	// adding timer value to current time so we can set an alarm time
	if (t.sec + t_in->sec > 60) {		// adding seconds
		if (t.min + t_in->min > 60) {		// adding minutes
			if (t.hr + t_in->hr > 24) {			// adding hours
				a.weekday = ((d.weekday + t_in->hr/24) % 7) + 1;		// bc weekday count starts from 1
			}
			a.hr = (t.hr + t_in->hr) % 24;
		}
		a.min = (t.min + t_in->min) % 60;
	}
	a.sec = (t.sec + t_in->sec) % 60;

	// setting RTC parameters
	salarmtime.Hours = a.hr;
	salarmtime.Minutes = a.min;
	salarmtime.Seconds = a.sec;
	salarmtime.TimeFormat = RTC_HOURFORMAT_24;
	salarmtime.SubSeconds = 0;
	salarmtime.SecondFraction = 0;
	salarmtime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	salarmtime.StoreOperation = RTC_STOREOPERATION_RESET;

	salarm.AlarmTime = salarmtime;
	salarm.AlarmMask = RTC_ALARMMASK_ALL;
	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	salarm.AlarmDateWeekDay = a.weekday;
	salarm.Alarm = RTC_ALARM_B;			// change if using different alarm

	// do nothing until done
//	while (HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BCD) != HAL_OK);
	HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BIN);

	runTimerDisplay(htim);
}

// ---- callbacks for interrupts ----
// used for alarm function in project
// meant to send signal to use motor
// change to use hw timer so signal is temporary
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
	// change pin to whatever's accessible
	// using PC0
//	switch (alarmMask) {
//		case RTC_ALARMMASK_DATEWEEKDAY: {
//			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
//			setAlarmWithMask(&watchAlarm, RTC_ALARMMASK_HOURS, hrtc);
//			alarmMask = RTC_ALARMMASK_HOURS;
//			break;
//		}
//		case RTC_ALARMMASK_HOURS: {
//			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
//			setAlarmWithMask(&watchAlarm, RTC_ALARMMASK_MINUTES, hrtc);
//			alarmMask = RTC_ALARMMASK_MINUTES;
//			break;
//		}
//		case RTC_ALARMMASK_MINUTES: {
//			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
//			setAlarmWithMask(&watchAlarm, RTC_ALARMMASK_SECONDS, hrtc);
//			alarmMask = RTC_ALARMMASK_SECONDS;
//			break;
//		}
//		case RTC_ALARMMASK_SECONDS: {
//			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
//		//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
//		//	HAL_Delay(500);			// does this work in interrupt/callback? might not
//		//	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
//			HAL_RTC_DeactivateAlarm(hrtc, RTC_ALARM_A);
//			break;
//		}
//		default: break;
//	}
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
	HAL_RTC_DeactivateAlarm(hrtc, RTC_ALARM_A);
}

// used to send interrupt every sec to print to display.
// hack?
// more efficient ways to do it?
//void HAL_RTC_AlarmBEventCallback(RTC_HandleTypeDef *hrtc) {
//	// print to screen
//	// set alarm for next sec
//	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
//	setAlarmB(hrtc);
//}

void HAL_RTC_AlarmBEventCallback(RTC_HandleTypeDef *hrtc) {
	// toggles pin on end of timer. clears alarm
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
	HAL_RTC_DeactivateAlarm(hrtc, RTC_ALARM_B);

	/*
	 * should run motor thing and update display to signal user
	 * also clear alarm
	 */
}
// ---- end of callbacks ----

// ---- clock get functions ----
// maybe needs subseconds?
void getTime(struct times *t, RTC_HandleTypeDef *hrtc) {
	RTC_TimeTypeDef stime;

	// programming manual says to read time after date. something shadow registers.
	// not done automatically in HAL
	HAL_RTC_GetTime(hrtc, &stime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(hrtc, NULL, RTC_FORMAT_BIN);

	t->hr = stime.Hours;
	t->min = stime.Minutes;
	t->sec = stime.Seconds;
}

void getDate(struct dates *d, RTC_HandleTypeDef *hrtc) {
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
void getDateTime(struct dates *d, struct times *t, RTC_HandleTypeDef *hrtc) {
	RTC_DateTypeDef sdate;
	RTC_TimeTypeDef stime;

	// programming manual says to read time after date. something shadow registers.
	// not done automatically in HAL
	HAL_RTC_GetTime(hrtc, &stime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(hrtc, &sdate, RTC_FORMAT_BIN);

	d->yr = sdate.Year > 50 ? sdate.Year+1900 : sdate.Year+2000;		// make assumptions on whether it's 19xx or 20xx
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
	getTime(&t, hrtc);

	setTextColor(ST77XX_WHITE);
	setBackgroundColor(ST77XX_BLACK);
	sprintf(str, "sec: %2d", t.sec);
	drawTextAt(0, 0, str, hspi);
	sprintf(str, "min: %2d", t.min);
	drawTextAt(0, 10, str, hspi);
	sprintf(str, "hr: %3d", t.hr);
	drawTextAt(0, 20, str, hspi);
}

void printDate(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	char str[40];		// problems when using only char*

	struct dates d;
	getDate(&d, hrtc);

	setTextColor(ST77XX_WHITE);
	setBackgroundColor(ST77XX_BLACK);
	sprintf(str, "year: %3d", d.yr);
	drawTextAt(0, 40, str, hspi);
	sprintf(str, "month: %4d", d.month);
	drawTextAt(0, 50, str, hspi);
	sprintf(str, "day: %2d", d.date);
	drawTextAt(0, 60, str, hspi);
	sprintf(str, "day: %2d", d.weekday);	// probably not gonna show nice since its an enum
	drawTextAt(0, 70, str, hspi);
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
	setTime(&t, hrtc);
	printDateTime(hrtc, hspi);
	HAL_Delay(1000);
	printDateTime(hrtc, hspi);

	HAL_Delay(1000);
	setDateTime(&d, &t, hrtc);
	printDateTime(hrtc, hspi);
	HAL_Delay(1000);
	printDateTime(hrtc, hspi);
}

void alarmTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	struct dates d;
	struct times t;

	getDateTime(&d, &t, hrtc);

//	if (t.sec > 60) t.min += 1;
//	t.sec = (t.sec+10) % 60;
//	struct alarmTimes a = {t.hr, t.min, t.sec, d.weekday};
	struct alarmTimes a = {0, 0, 15, RTC_WEEKDAY_MONDAY};

	HAL_Delay(1000);

	char str[40];
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, hspi);
	sprintf(str, "%2u:%2u:%2u %2u", a.hr, a.min, a.sec, a.weekday);
	drawTextAt(0, 10, str, hspi);

	setAlarm(&a, hrtc);

	HAL_Delay(10000);
	getDateTime(&d, &t, hrtc);
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, hspi);
}
