/*
 * file for initializing and working with the real-time clock
 * and low-power/general purpose timers for time-keeping functionality
 *
 * called in main(). uses global variables?
 */


#include "clocks.h"

// set rtc time. uses perosnal struct as arg
void setTime(struct times *t) {
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
	HAL_RTC_SetTime(&hrtc, &stime, RTC_FORMAT_BIN);

	runClockDisplay(&htim22);
}

// set rtc date. uses personal struct
void setDate(struct dates *d) {
	// ---- date ----
	RTC_DateTypeDef sdate = {0};

	sdate.Month = d->month;
	sdate.Date = d->date;
	sdate.WeekDay = d->weekday;
	sdate.Year = d->yr % 100; 		// set only between 0-99. part of the library (!?)

	HAL_RTC_SetDate(&hrtc, &sdate, RTC_FORMAT_BIN);

	runClockDisplay(&htim22);
}

void setDateTime(struct dates *d, struct times *t) {
	setDate(d);
	setTime(t);
}

// for time of day+week
void setAlarm(struct alarmTimes *a) {
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
//	salarm.AlarmMask = RTC_ALARMMASK_ALL;
	salarm.AlarmMask = RTC_ALARMMASK_NONE;
	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	salarm.AlarmDateWeekDay = a->weekday;
	salarm.Alarm = RTC_ALARM_A;			// change if using different alarm

	// do nothing until done
	HAL_RTC_SetAlarm_IT(&hrtc, &salarm, RTC_FORMAT_BIN);
}

// set alarm for timer function of watch project
// using RTC alarm hardware
void setTimer(struct times *t_in) {
	RTC_AlarmTypeDef salarm = {0};
	RTC_TimeTypeDef salarmtime = {0};

	// set global variables to hold value being set
	watchTimer = *t_in;
	watchTimerSeconds = t_in->sec + t_in->min*60 + t_in->hr*3600;

	// pull current RTC time
	struct dates d;
	struct times t;
	getDateTime(&d, &t);

	struct alarmTimes a = {0};
	uint8_t s,m,h,w;

	// adding timer value to current time so we can set an alarm time
	// REDO THIS ADDER BC ITS NOT RIGHT
//	if (t.sec + t_in->sec > 60) {		// adding seconds
//		if (t.min + t_in->min > 60) {		// adding minutes
//			if (t.hr + t_in->hr > 24) {			// adding hours
//				a.weekday = ((d.weekday + t_in->hr/24) % 7) + 1;		// bc weekday count starts from 1
//			}
//			a.hr = (t.hr + t_in->hr) % 24;
//		}
//		a.min = (t.min + t_in->min) % 60;
//	}
//	a.sec = (t.sec + t_in->sec) % 60;
//
	s = t.sec + t_in->sec;
	m = t.min + t_in->min + s/60;
	h = t.hr + t_in->hr + m/60;
	w = d.weekday + h/24;
	a.sec = s % 60;
	a.min = m % 60;
	a.hr = h % 24;
	a.weekday = (w-1) % 7 + 1;
//	a.sec = t_in->sec;
//	a.min = t_in->min;
//	a.hr = t_in->hr;
//	a.weekday = d.weekday;

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
	salarm.AlarmMask = RTC_ALARMMASK_NONE;
	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	salarm.AlarmDateWeekDay = a.weekday;
	salarm.Alarm = RTC_ALARM_B;			// change if using different alarm

	HAL_RTC_SetAlarm_IT(&hrtc, &salarm, RTC_FORMAT_BIN);

	runTimerDisplay();
}

// ---- callbacks for interrupts ----
// used for alarm function in project
// meant to send signal to use motor
// change to use hw timer so signal is temporary
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
	// change pin to whatever's accessible
	// using PC0
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
	HAL_RTC_DeactivateAlarm(hrtc, RTC_ALARM_A);
	alarmRunning = 0;
	updateAlarm = 1;
//	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
//	HAL_Delay(500);			// does this work in interrupt/callback? might not
//	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
}

void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc) {
	// toggles pin on end of timer. clears alarm
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
	HAL_RTC_DeactivateAlarm(hrtc, RTC_ALARM_B);
	timerRunning = 0;
	updateTimer = 1;
	/*
	 * should run motor thing and update display to signal user
	 * also clear alarm
	 */
}
// ---- end of callbacks ----

// ---- clock get functions ----
// maybe needs subseconds?
void getTime(struct times *t) {
	RTC_TimeTypeDef stime;

	// programming manual says to read time after date. something shadow registers.
	// not done automatically in HAL
	HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, NULL, RTC_FORMAT_BIN);

	t->hr = stime.Hours;
	t->min = stime.Minutes;
	t->sec = stime.Seconds;
}

void getDate(struct dates *d) {
	RTC_DateTypeDef sdate;

	// programming manual says to read time after date. something shadow registers.
	// not done automatically in HAL
	HAL_RTC_GetTime(&hrtc, NULL, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);

	d->yr = sdate.Year > 50 ? sdate.Year+1900 : sdate.Year+2000;
	d->month = sdate.Month;
	d->date = sdate.Date;
	d->weekday = sdate.WeekDay;
}

// not using getDate and getTime for efficiency (?)
void getDateTime(struct dates *d, struct times *t) {
	RTC_DateTypeDef sdate;
	RTC_TimeTypeDef stime;

	// programming manual says to read time after date. something shadow registers.
	// not done automatically in HAL
	HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);

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
void printTime() {
	char str[40];		// problems when using only char*

	struct times t;
	getTime(&t);

	setTextColor(ST77XX_WHITE);
	setBackgroundColor(ST77XX_BLACK);
	sprintf(str, "sec: %2d", t.sec);
	drawTextAt(0, 0, str, &hspi1);
	sprintf(str, "min: %2d", t.min);
	drawTextAt(0, 10, str, &hspi1);
	sprintf(str, "hr: %3d", t.hr);
	drawTextAt(0, 20, str, &hspi1);
}

void printDate() {
	char str[40];		// problems when using only char*

	struct dates d;
	getDate(&d);

	setTextColor(ST77XX_WHITE);
	setBackgroundColor(ST77XX_BLACK);
	sprintf(str, "year: %3d", d.yr);
	drawTextAt(0, 40, str, &hspi1);
	sprintf(str, "month: %4d", d.month);
	drawTextAt(0, 50, str, &hspi1);
	sprintf(str, "day: %2d", d.date);
	drawTextAt(0, 60, str, &hspi1);
	sprintf(str, "day: %2d", d.weekday);	// probably not gonna show nice since its an enum
	drawTextAt(0, 70, str, &hspi1);
}

void printDateTime() {
	printTime();
	printDate();
}
// ---- end of clock print functions ----

// tests clock functions. assumes SPI display using TFT_display.c is available
void clockTest() {
	struct times t = {1, 1, 1};
	struct dates d = {19, 11, 13};

	HAL_Delay(1000);
	printDateTime();
	HAL_Delay(2000);
	printDateTime();

	HAL_Delay(1000);
	setTime(&t);
	printDateTime();
	HAL_Delay(1000);
	printDateTime();

	HAL_Delay(1000);
	setDateTime(&d, &t);
	printDateTime();
	HAL_Delay(1000);
	printDateTime();
}

void alarmTest() {
	struct dates d;
	struct times t;

	getDateTime(&d, &t);

//	if (t.sec > 60) t.min += 1;
//	t.sec = (t.sec+10) % 60;
//	struct alarmTimes a = {t.hr, t.min, t.sec, d.weekday};
//	struct alarmTimes a = {0, 1, 0, RTC_WEEKDAY_MONDAY};
	struct alarmTimes a = {0, 0, 30, RTC_WEEKDAY_MONDAY};

	HAL_Delay(1000);

	char str[40];
	clearScreen(ST77XX_WHITE, &hspi1);
	setTextColor(ST77XX_BLACK);
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, &hspi1);
	sprintf(str, "%2u:%2u:%2u %2u", a.hr, a.min, a.sec, a.weekday);
	drawTextAt(0, 10, str, &hspi1);

	setAlarm(&a);

	HAL_Delay(30000);
	getDateTime(&d, &t);
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, &hspi1);

	a.sec += 20;
	clearScreen(ST77XX_WHITE, &hspi1);
	setTextColor(ST77XX_BLACK);
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, &hspi1);
	sprintf(str, "%2u:%2u:%2u %2u", a.hr, a.min, a.sec, a.weekday);
	drawTextAt(0, 10, str, &hspi1);

	setAlarm(&a);

	HAL_Delay(30000);
	getDateTime(&d, &t);
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, &hspi1);
}

// set a timer for a short time (10s) and watch it go
void timerTest() {
	struct times timerTime = {0, 0, 20};

	setTimer(&timerTime);

	// print current time and timer value set
	struct dates currentDate;
	struct times currentTime;
	getDateTime(&currentDate, &currentTime);

	char str[40];
	sprintf(str, "current: %2u:%2u:%2u", currentTime.hr, currentTime.min, currentTime.sec);
	drawTextAt(0, 0, str, &hspi1);
	sprintf(str, "       : %2u:%2u:%2u", currentDate.month, currentDate.date, currentDate.yr);
	drawTextAt(0, 10, str, &hspi1);
	sprintf(str, "timer: %2u:%2u:%2u", timerTime.hr, timerTime.min, timerTime.sec);
	drawTextAt(0, 20, str, &hspi1);

	HAL_Delay(20000);
	getDateTime(&currentDate, &currentTime);
	sprintf(str, "current: %2u:%2u:%2u", currentTime.hr, currentTime.min, currentTime.sec);
	drawTextAt(0, 0, str, &hspi1);
	sprintf(str, "       : %2u:%2u:%2u", currentDate.month, currentDate.date, currentDate.yr);
	drawTextAt(0, 10, str, &hspi1);


	timerTime.sec+=20;
	setTimer(&timerTime);

	// print current time and timer value set
	getDateTime(&currentDate, &currentTime);

	sprintf(str, "current: %2u:%2u:%2u", currentTime.hr, currentTime.min, currentTime.sec);
	drawTextAt(0, 0, str, &hspi1);
	sprintf(str, "       : %2u:%2u:%2u", currentDate.month, currentDate.date, currentDate.yr);
	drawTextAt(0, 10, str, &hspi1);
	sprintf(str, "timer: %2u:%2u:%2u", timerTime.hr, timerTime.min, timerTime.sec);
	drawTextAt(0, 20, str, &hspi1);

	HAL_Delay(40000);
	getDateTime(&currentDate, &currentTime);
	sprintf(str, "current: %2u:%2u:%2u", currentTime.hr, currentTime.min, currentTime.sec);
	drawTextAt(0, 0, str, &hspi1);
	sprintf(str, "       : %2u:%2u:%2u", currentDate.month, currentDate.date, currentDate.yr);
	drawTextAt(0, 10, str, &hspi1);
}
