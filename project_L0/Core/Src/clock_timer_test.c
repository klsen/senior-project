// test file for clocks.c and timers.c

#include "clock_timer_test.h"

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
