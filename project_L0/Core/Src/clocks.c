/*
 * file for initializing and working with the real-time clock
 * only for time-keeping and alarm triggering functionality
 *
 * called in main(). uses global variables? might change back
 */

#include "clocks.h"

// set rtc time. uses perosnal struct as arg
// assert members not null for set functions?
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

//	runClockDisplay(&htim22);
}

// set rtc date. uses personal struct
void setDate(struct dates *d, RTC_HandleTypeDef *hrtc) {
	// ---- date ----
	RTC_DateTypeDef sdate = {0};

	sdate.Month = d->month;
	sdate.Date = d->date;
	sdate.WeekDay = weekdayCalculator(d->yr, d->month, d->date) % 7;
	sdate.Year = d->yr % 100; 		// set only between 0-99. part of the library (!?)

	HAL_RTC_SetDate(hrtc, &sdate, RTC_FORMAT_BIN);

//	runClockDisplay(&htim22);
}

void setDateTime(struct dates *d, struct times *t, RTC_HandleTypeDef *hrtc) {
	setDate(d, hrtc);
	setTime(t, hrtc);
}

// for time of day+week
void setAlarm(struct alarmTimes *a, RTC_HandleTypeDef *hrtc) {
	RTC_AlarmTypeDef salarm = {0};		// is there a problem with using pointers instead?
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
	salarm.AlarmMask = RTC_ALARMMASK_NONE;
	salarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	salarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	salarm.AlarmDateWeekDay = a->weekday;
	salarm.Alarm = RTC_ALARM_A;			// change if using different alarm

	// do nothing until done
	HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BIN);
}

// set an alarm for the next second.
// for triggering display updates.
// uses rtc weekday. should have weekday calculator integrated before using
void setClockAlarm(RTC_HandleTypeDef *hrtc) {
	RTC_AlarmTypeDef salarm = {0};			// malloc if using pointers
	RTC_TimeTypeDef salarmtime = {0};

	struct dates currentDate = {0};
	struct times currentTime = {0};

	getDateTime(&currentDate, &currentTime, hrtc);

	struct alarmTimes a = {0};
	uint8_t s,m,h,w;
	s = currentTime.sec + 1;
	m = currentTime.min + s/60;
	h = currentTime.hr + m/60;
	w = currentDate.weekday + h/24;
	a.sec = s % 60;
	a.min = m % 60;
	a.hr = h % 24;
	a.weekday = (w-1) % 7 + 1;

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

	// do nothing until done
	HAL_RTC_SetAlarm_IT(hrtc, &salarm, RTC_FORMAT_BIN);
}

// ---- callbacks for interrupts ----
// used for alarm function in project
// meant to send signal to use motor
// change to use hw timer so signal is temporary
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
	// change pin to whatever's accessible
	// using PC0
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
	isAlarmDone = 1;
	updateFace.alarm = 1;
}

// used to trigger display refresh every second. used because then it's synchronous with RTC updates
void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc) {
	updateFace.clock = 1;
	setClockAlarm(hrtc);
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

// ---- RTC calibration function ----
// calibVal should be given in drift/day in seconds
// calibration output on PC13. problems with using pins together with alarm?
void setRTCCalibration(int calibVal, RTC_HandleTypeDef *hrtc) {
	uint16_t calm = 0;
	double temp;
	// need to recalculate the bounds
	if (calibVal == 0) return;
	else if (calibVal < 0) {		// drift offset is negative. need to slow rtc down
		if (calibVal < -42) {		// bounds checking. just set to max
			HAL_RTCEx_SetSmoothCalib(hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_RESET, 0x1FF);
		}
		else {
			// math for setting CALM 9-bit register in RTC. formula in notes and in L0 programming reference manual
			temp = -calibVal*32768/86400*32;		// possible overflow when doing math, so reordering
			calm = temp;
			HAL_RTCEx_SetSmoothCalib(hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_RESET, calm);
		}
	}
	else {
		if (calibVal > 42) { 		// drift offset is positive. need to speed rtc up
			HAL_RTCEx_SetSmoothCalib(hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_SET, 0x1FF);
		}
		else {
			// math
			temp = 512-calibVal*32768/86400*32;
			calm = temp;
			HAL_RTCEx_SetSmoothCalib(hrtc, RTC_SMOOTHCALIB_PERIOD_32SEC, RTC_SMOOTHCALIB_PLUSPULSES_SET, calm);
		}
	}
}
// ---- end of RTC calibration function ----

// ---- converters and calculators ----
uint32_t timeToSeconds(struct times *t) {
	return t->sec + t->min*60 + t->hr*3600;
}

void secondsToTime(struct times *t, uint32_t seconds) {
	t->hr = seconds / 3600;
	seconds %= 3600;
	t->min = seconds / 60;
	seconds %= 60;
	t->sec = seconds;
}
/*
 * returns weekday from given year, month, and day.
 * algorithm stolen from wikipedia.
 * weekdays is 0-6, with 0 being sunday. hal uses 1=monday, 7=sunday - just call with % 7 to integrate with hal
 * months given in 1-12, with 1 being january. hal uses the same setup
 * rtc represents years with last 2 digits only. make sure year has all 4 numbers
 * should be accurate for any gregorian date
 */
uint8_t weekdayCalculator(uint16_t year, uint8_t month, uint8_t day) {
	static uint8_t table[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	if (month < 3) year--;
	uint16_t temp = (year + year/4 - year/100 + year/400 + table[month-1] + day) % 7;
	return temp;
}

// calculator for number of days in a month given a month and accounting for leap years
// assumes month is 1-12, 1=january, 12=december
uint8_t maxDaysInMonth(uint8_t month, uint16_t year) {
	if (month == 0 || month > 12) return 0;		// bounds checking

	// not using built-in defines, because they're in BCD
	if (month == 1  ||		// january
		month == 3  ||		// march
		month == 5  ||		// may
		month == 7  ||		// july
		month == 8  ||		// august
		month == 10 ||		// october
		month == 12) {		// december
		return 31;
	}
	else if (month == 4 ||	// april
			 month == 6 ||	// june
			 month == 9 ||	// september
			 month == 11) {	// november
		return 30;
	}

	// february/leap year calculator
	// leap year for every 4th year, but every 100th year is not a leap year except on every 400th year
	// ex. 2020 is a leap year, 2100 is not a leap year, 2000 is a leap year.
	else if (year % 400 == 0) return 29;
	else if (year % 100 == 0) return 28;
	else if (year % 4 == 0) return 29;
	else return 28;
}
// ---- end of converters and calculators ----
