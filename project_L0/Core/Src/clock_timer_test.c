// test file for clocks.c and timers.c

#include "clock_timer_test.h"

// ---- clock print functions ----
// print functions for RTC
// assumes we're using SPI display and file TFT_display.c
// pulls date and time structs automatically to only print current time in RTC
// not really needed anymore, almost pretty much a test function
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
//	struct alarmTimes a = {0, 1, 0, RTC_WEEKDAY_MONDAY};
	struct alarmTimes a = {0, 0, 30, RTC_WEEKDAY_MONDAY};

	HAL_Delay(1000);

	char str[40];
	clearScreen(ST77XX_WHITE, hspi);
	setTextColor(ST77XX_BLACK);
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, hspi);
	sprintf(str, "%2u:%2u:%2u %2u", a.hr, a.min, a.sec, a.weekday);
	drawTextAt(0, 10, str, hspi);

	setAlarm(&a, hrtc);

	HAL_Delay(30000);
	getDateTime(&d, &t, hrtc);
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, hspi);

	a.sec += 20;
	clearScreen(ST77XX_WHITE, hspi);
	setTextColor(ST77XX_BLACK);
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, hspi);
	sprintf(str, "%2u:%2u:%2u %2u", a.hr, a.min, a.sec, a.weekday);
	drawTextAt(0, 10, str, hspi);

	setAlarm(&a, hrtc);

	HAL_Delay(30000);
	getDateTime(&d, &t, hrtc);
	sprintf(str, "%2u:%2u:%2u %2u", t.hr, t.min, t.sec, d.weekday);
	drawTextAt(0, 0, str, hspi);
}

// set a timer for a short time (10s) and watch it go
//void timerTest() {
//	struct times timerTime = {0, 0, 20};
//
//	setTimer(&timerTime);
//
//	// print current time and timer value set
//	struct dates currentDate;
//	struct times currentTime;
//	getDateTime(&currentDate, &currentTime);
//
//	char str[40];
//	sprintf(str, "current: %2u:%2u:%2u", currentTime.hr, currentTime.min, currentTime.sec);
//	drawTextAt(0, 0, str, hspi);
//	sprintf(str, "       : %2u:%2u:%2u", currentDate.month, currentDate.date, currentDate.yr);
//	drawTextAt(0, 10, str, hspi);
//	sprintf(str, "timer: %2u:%2u:%2u", timerTime.hr, timerTime.min, timerTime.sec);
//	drawTextAt(0, 20, str, hspi);
//
//	HAL_Delay(20000);
//	getDateTime(&currentDate, &currentTime);
//	sprintf(str, "current: %2u:%2u:%2u", currentTime.hr, currentTime.min, currentTime.sec);
//	drawTextAt(0, 0, str, hspi);
//	sprintf(str, "       : %2u:%2u:%2u", currentDate.month, currentDate.date, currentDate.yr);
//	drawTextAt(0, 10, str, hspi);
//
//
//	timerTime.sec+=20;
//	setTimer(&timerTime);
//
//	// print current time and timer value set
//	getDateTime(&currentDate, &currentTime);
//
//	sprintf(str, "current: %2u:%2u:%2u", currentTime.hr, currentTime.min, currentTime.sec);
//	drawTextAt(0, 0, str, hspi);
//	sprintf(str, "       : %2u:%2u:%2u", currentDate.month, currentDate.date, currentDate.yr);
//	drawTextAt(0, 10, str, hspi);
//	sprintf(str, "timer: %2u:%2u:%2u", timerTime.hr, timerTime.min, timerTime.sec);
//	drawTextAt(0, 20, str, hspi);
//
//	HAL_Delay(40000);
//	getDateTime(&currentDate, &currentTime);
//	sprintf(str, "current: %2u:%2u:%2u", currentTime.hr, currentTime.min, currentTime.sec);
//	drawTextAt(0, 0, str, hspi);
//	sprintf(str, "       : %2u:%2u:%2u", currentDate.month, currentDate.date, currentDate.yr);
//	drawTextAt(0, 10, str, hspi);
//}
