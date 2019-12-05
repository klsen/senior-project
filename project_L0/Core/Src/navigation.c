/*
 * File for handling navigation between different apps/interfaces.
 * uses global flags (concurrency problem?).
 */

#include "navigation.h"

// use static for timer/alarm/stopwatch variables, changeface, and face used.
// needed only in updatedisplay/buttons, but each func shares these variables
static struct clockVariables clockVars;
static struct timerVariables timerVars;
static struct alarmVariables alarmVars;
static struct stopwatchVariables stopwatchVars;
static uint8_t isFaceBeingChanged;
static uint8_t faceOnDisplay;

const char* weekdayNames[8] = {
	"",			// padding so i can avoid dealing with out-of-bounds access
	"mon  ",
	"tues ",
	"wed  ",
	"thurs",
	"fri  ",
	"sat  ",
	"sun  "
};

const char* monthNames[13] = {
	"",			// month names also start with 1
	"jan ",
	"feb ",
	"mar ",
	"apr ",
	"may ",
	"jun ",
	"jul ",
	"aug ",
	"sept",
	"oct ",
	"nov ",
	"dec "
};

void updateWithButtons() {
	/* program flow:
	 *   check current face used
	 *   check current variables and check button pressed
	 */
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);		// should run for any button

	// button 1 changes the face on screen.
	if (buttons.is1Pressed) {
		buttons.is1Pressed = 0;
		faceOnDisplay = (faceOnDisplay + 1) % NUM_FACES;
		isFaceBeingChanged = 1;
		switch (faceOnDisplay) {
			case faceClock: updateFace.clock = 1; break;
			case faceTimer: updateFace.timer = 1; break;
			case faceAlarm: updateFace.alarm = 1; break;
			case faceStopwatch: updateFace.stopwatch = 1; break;
			default: break;
		}
	}

	if (faceOnDisplay == faceClock) updateClockState();
	else if (faceOnDisplay == faceTimer) updateTimerState();
	else if (faceOnDisplay == faceAlarm) updateAlarmState();
	else if (faceOnDisplay == faceStopwatch) updateStopwatchState();
}

/*
 * does various things when clock is the face on screen.
 *
 * in default mode:
 *   button 2 does nothing
 *   button 3 does nothing
 *   button 4 is used to change to mode where clock is being set to some other time
 *
 * in setting mode:
 *   button 2 changes value up
 *   button 3 changes value down
 *   button 4 changes field being set. changes between min, hr, year, month, and day. once it finishes cycling through it once,
 *     the clock is updated and we revert back to default mode.
 *
 * notes:
 *   make date setting more robust (invalidate date entries when that day of month doesn't exist or just change modulo)
 */
void updateClockState() {
	// change fields up, do nothing if not setting clock
	if (buttons.is2Pressed && clockVars.isBeingSet) {
		buttons.is2Pressed = 0;
		updateFace.clock = 1;
		switch (clockVars.fieldBeingSet) {
			case 1: clockVars.timeToSet->min = (clockVars.timeToSet->min+1) % 60; break;
			case 2: clockVars.timeToSet->hr = (clockVars.timeToSet->hr+1) % 24; break;
			case 3: clockVars.dateToSet->yr++; break;		// supposed to be between large numbers. no need for bounds checking
			case 4: clockVars.dateToSet->month = (clockVars.dateToSet->month+1) % 12 + 1; break;
			case 5: clockVars.dateToSet->date = (clockVars.dateToSet->date+1) % maxDaysInMonth(clockVars.dateToSet->month, clockVars.dateToSet->yr); break;		// make more robust?
			default: break;
		}
	}
	// change fields down, do nothing if not setting clock
	if (buttons.is3Pressed && clockVars.isBeingSet) {
		buttons.is3Pressed = 0;
		updateFace.clock = 1;
		switch (clockVars.fieldBeingSet) {
			case 1:
				if (clockVars.timeToSet->min == 0) clockVars.timeToSet->min = 59;
				else clockVars.timeToSet->min--;
				break;
			case 2:
				if (clockVars.timeToSet->hr == 0) clockVars.timeToSet->hr = 23;
				else clockVars.timeToSet->hr--;
				break;
			case 3: clockVars.dateToSet->yr--; break;		// supposed to be from 1950-2050. no need to do bounds checking
			case 4: clockVars.dateToSet->month = clockVars.dateToSet->month == 1 ? 12 : clockVars.dateToSet->month-1; break;
				if (clockVars.dateToSet->month == RTC_MONTH_JANUARY) clockVars.dateToSet->month = RTC_MONTH_DECEMBER;
				else clockVars.dateToSet->month--;
			case 5:
				if (clockVars.dateToSet->date == 0) clockVars.dateToSet->date = maxDaysInMonth(clockVars.dateToSet->month, clockVars.dateToSet->yr);
				else clockVars.dateToSet->date--;
				break;
			default: break;
		}
	}
	// switches between setting mode and default mode. changes between different clock fields
	if (buttons.is4Pressed) {
		buttons.is4Pressed = 0;
		updateFace.clock = 1;
		clockVars.fieldBeingSet = (clockVars.fieldBeingSet + 1) % (NUM_CLOCKFIELDS + 1);
		if (clockVars.fieldBeingSet != 0) {
			clockVars.isBeingSet = 1;

			// should pull current time when first entering setting mode
			if (clockVars.fieldBeingSet == 1) getDateTime(clockVars.dateToSet, clockVars.timeToSet);
		}
		else {
			clockVars.isBeingSet = 0;

			// second set to 0, weekday ignored
			setDateTime(clockVars.dateToSet, clockVars.timeToSet);
		}
	}
	// checks on clock set for other buttons here (what did this note mean??)
}

/*
 * does various things when timer is the face on screen.
 *
 * in default mode:
 *   button 2 does nothing
 *   button 3 does nothing
 *   button 4 changes to setting mode
 *
 * in not running mode (timer value has been set to something):
 *   button 2 starts timer
 *   button 3 does nothing
 *   button 4 clears timer and returns to default mode
 *
 * in running mode:
 *   button 2 does nothing
 *   button 3 pauses timer (not included in 1st implementation) and moves to not running mode
 *   button 4 stops and clears timer and returns to default mode
 *
 * in setting mode:
 *   button 2 changes value up
 *   button 3 changes value down
 *   button 4 changes field being set. changes between sec, min, hr. returns to default mode after
 *     cycling through fields once.
 *
 * notes:
 *   using rtc alarm now, but not subseconds, so timer is off a little (eg. when setting timer to 1s and rtc is halfway to the next
 *     second, timer only runs for half second.)
 *     also not sure how to implement pause using rtc (alarm value has to change depending on how long timer is paused for)
 *   might need to change to using only hardware timer for this instead of rtc because of problems listed above
 *   insert a few more functions into this (those that need to use the hardware)
 */
void updateTimerState() {
	if (timerVars.isBeingSet) {
		if (buttons.is2Pressed) {
			buttons.is2Pressed = 0;
			updateFace.timer = 1;

			// set field up
			switch (timerVars.fieldBeingSet) {
				case 1: timerVars.timeToSet->sec = (timerVars.timeToSet->sec+1) % 60; break;
				case 2: timerVars.timeToSet->min = (timerVars.timeToSet->min+1) % 60; break;
				case 3: timerVars.timeToSet->hr = (timerVars.timeToSet->hr+1) % 24; break;
				default: break;
			}
		}
		if (buttons.is3Pressed) {
			buttons.is3Pressed = 0;
			updateFace.timer = 1;

			// set field down
			switch (timerVars.fieldBeingSet) {
				case 1:
					if (timerVars.timeToSet->sec == 0) timerVars.timeToSet->sec = 59;
					else timerVars.timeToSet->sec--;
					break;
				case 2:
					if (timerVars.timeToSet->min == 0) timerVars.timeToSet->min = 59;
					else timerVars.timeToSet->min--;
					break;
				case 3:
					if (timerVars.timeToSet->hr == 0) timerVars.timeToSet->hr = 23;
					else timerVars.timeToSet->hr--;
					break;
				default: break;
			}
		}
	}
	// not done
	else if (timerVars.isSet) {
		if (buttons.is2Pressed && isTimerRunning == 0) {
			buttons.is2Pressed = 0;
			updateFace.timer = 1;
			// start timer
			isTimerRunning = 1;
		}
		if (buttons.is3Pressed && isTimerRunning) {
			buttons.is3Pressed = 0;
			updateFace.timer = 1;
			// pause timer
			isTimerRunning = 0;
		}
		if (buttons.is4Pressed) {
			buttons.is4Pressed = 0;
			updateFace.timer = 1;

			// stop and clear timer
			timerVars.isSet = 0;
			isTimerRunning = 0;
		}
	}
	// not done? might be done (other buttons start/stop timer)
	else if (buttons.is4Pressed) {
		buttons.is4Pressed = 0;
		updateFace.timer = 1;

		// change field/mode
		timerVars.fieldBeingSet = (timerVars.fieldBeingSet + 1) % (NUM_TIMERFIELDS + 1);
		if (timerVars.fieldBeingSet != 0) {
			timerVars.isBeingSet = 1;
			timerVars.isSet = 0;

			// set temp fields to 0 when first entering setting mode
			if (timerVars.fieldBeingSet == 1) {
				timerVars.timeToSet->sec = 0;
				timerVars.timeToSet->min = 0;
				timerVars.timeToSet->hr = 0;
			}
		}
		else {
			timerVars.isBeingSet = 0;
			timerVars.isSet = 1;
			// setTimer(&timerVars.timeToSet);
		}
	}
}

/*
 * does various things when alarm is the face on screen.
 * WE GONNA BE CHANGING THINGS
 *
 * in default mode:
 *   button 2 does nothing
 *   button 3 does nothing
 *   button 4 changes to setting mode
 *
 * in running mode:
 *   button 2 does nothing
 *   button 3 does nothing
 *   button 4 stops and clears alarm and returns to default mode
 *
 * in setting mode:
 *   button 2 changes value up
 *   button 3 changes value down
 *   button 4 changes field being set. changes between sec, min, hr. returns to default mode after
 *     cycling through fields once.
 *
 * notes:
 *   should change to make it possible to have multiple alarms
 *   also pick alarms that repeat and alarms that don't
 *   need to make changes to ui to make this happen
 *   currently just does old behavior (only 1 alarm)
 */
void updateAlarmState() {
	if (buttons.is2Pressed && alarmVars.isBeingSet) {
		buttons.is2Pressed = 0;
		updateFace.alarm = 1;

		// change fields up
		switch (alarmVars.fieldBeingSet) {
			case 1: alarmVars.alarmToSet->sec = (alarmVars.alarmToSet->sec + 1) % 60; break;
			case 2: alarmVars.alarmToSet->min = (alarmVars.alarmToSet->min + 1) % 60; break;
			case 3: alarmVars.alarmToSet->hr = (alarmVars.alarmToSet->hr + 1) % 24; break;
			case 4: alarmVars.alarmToSet->weekday = (alarmVars.alarmToSet->weekday + 1) % 7 + 1; break;
			default: break;
		}
	}
	if (buttons.is3Pressed && alarmVars.isBeingSet) {
		buttons.is3Pressed = 0;
		updateFace.alarm = 1;

		// change fields down
		switch (alarmVars.fieldBeingSet) {
			case 1:
				if (alarmVars.alarmToSet->sec == 0) alarmVars.alarmToSet->sec = 59;
				else alarmVars.alarmToSet->sec--;
				break;
			case 2:
				if (alarmVars.alarmToSet->min == 0) alarmVars.alarmToSet->min = 59;
				else alarmVars.alarmToSet->min--;
				break;
			case 3:
				if (alarmVars.alarmToSet->hr == 0) alarmVars.alarmToSet->hr = 23;
				else alarmVars.alarmToSet->hr--;
				break;
			case 4:
				if (alarmVars.alarmToSet->weekday == RTC_WEEKDAY_MONDAY) alarmVars.alarmToSet->weekday = RTC_WEEKDAY_SUNDAY;
				else alarmVars.alarmToSet->weekday--;
				break;
			default: break;
		}
	}
	if (buttons.is4Pressed) {
		buttons.is4Pressed = 0;
		updateFace.alarm = 1;

		if (isAlarmRunning == 0) {
			// toggle between fields
			alarmVars.fieldBeingSet = (alarmVars.fieldBeingSet + 1) % (NUM_ALARMFIELDS + 1);
			if (alarmVars.fieldBeingSet != 0) {
				alarmVars.isBeingSet = 1;
				if (alarmVars.fieldBeingSet == 1) {
					struct dates *d;
					struct times *t;
					getDateTime(d, t);
					alarmVars.alarmToSet->sec = t->sec;
					alarmVars.alarmToSet->min = t->min;
					alarmVars.alarmToSet->hr = t->hr;
					alarmVars.alarmToSet->weekday = d->weekday;
				}
			}
			else {
				alarmVars.isBeingSet = 0;
				isAlarmRunning = 1;
				setAlarm(alarmVars.alarmToSet);
			}
		}
		else {
			// stop and clear alarm hw
			isAlarmRunning = 0;
			HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
		}
	}
}

/*
 * does various things when stopwatch is the face on screen.
 *
 * in default mode:
 *   button 2 starts stopwatch
 *   button 3 does nothing
 *   button 4 does nothing
 *
 * in running mode:
 *   button 2 pauses stopwatch and moves to not running mode
 *   button 3 captures stopwatch for lap
 *   button 4 stops and clears stopwatch and returns to default mode
 *
 * in not running mode:
 *   button 2 starts stopwatch and moves to running mode
 *   button 3 captures stopwatch for lap
 *   button 4 clears stopwatch and returns to default mode
 *
 * notes:
 *   using lptim now, but might need to change to use other timer as lptim might be used by adc
 *     to take regular measurements of the battery
 *   would just have to modify functions in timers.c
 */
void updateStopwatchState() {
	if (buttons.is2Pressed) {	// start/stop
		buttons.is2Pressed = 0;
		updateFace.stopwatch = 1;

		if (isStopwatchRunning == 0) {
			isStopwatchRunning = 1;
			runStopwatch();
		}
		else {
			isStopwatchRunning = 0;
			pauseStopwatch();
		}
	}
	if (buttons.is3Pressed) {
		buttons.is3Pressed = 0;
		updateFace.stopwatch = 1;

		// pull data and set lap
		stopwatchVars.lapPrev = stopwatchVars.lapCurrent;
		stopwatchVars.lapCurrent = stopwatchCNT;		// did this variable get changed to something else?
	}
	if (buttons.is4Pressed) {
		buttons.is4Pressed = 0;
		updateFace.stopwatch = 1;

		// clear stopwatch hw
		isStopwatchRunning = 0;
		clearStopwatch();
	}
}

// update screen based on global variables
// going in main, so it's executing in a while loop
//   software interrupt on flag so that this doesn't run all the time?
void updateDisplay(SPI_HandleTypeDef *hspi) {
	if (isFaceBeingChanged == 1) {
		isFaceBeingChanged = 0;

		// drawing titles and boxes that won't be rewritten during normal operation within
		// a specific face (titles and buttons)
		if (faceOnDisplay == faceClock) {
			clearScreen(ST77XX_CYAN, hspi);
			drawTitle("clock", hspi);
		}
		if (faceOnDisplay == faceTimer) {
			clearScreen(ST77XX_GREEN, hspi);
			drawTitle("timer", hspi);
		}
		if (faceOnDisplay == faceAlarm) {
			clearScreen(ST77XX_MAGENTA, hspi);
			drawTitle("alarm", hspi);
		}
		if (faceOnDisplay == faceStopwatch) {
			clearScreen(ST77XX_YELLOW, hspi);
			drawTitle("stopwatch", hspi);
		}

		drawButton(WIDTH/4-5, HEIGHT-20, hspi);
		drawButton(WIDTH/2-5, HEIGHT-20, hspi);
		drawButton(WIDTH/4*3-5, HEIGHT-20, hspi);
	}

	// update clock face
	if (faceOnDisplay == faceClock) {
		if (updateFace.clock == 1) {
			updateFace.clock = 0;
			updateClockDisplay(hspi);
		}
	}
	// update timer face
	else if (faceOnDisplay == faceTimer) {
		if (updateFace.timer == 1) {
			updateFace.timer = 0;
			updateTimerDisplay(hspi);
		}
	}
	// update alarm face
	else if (faceOnDisplay == faceAlarm) {
		if (updateFace.alarm == 1) {
			updateFace.alarm = 0;
			updateAlarmDisplay(hspi);
		}
	}
	// update stopwatch face
	else if (faceOnDisplay == faceStopwatch) {
		if (updateFace.stopwatch == 1) {
			updateFace.stopwatch = 0;
			updateStopwatchDisplay(hspi);
		}
	}
}

void updateClockDisplay(SPI_HandleTypeDef *hspi) {
	char str[40];
	struct dates *currentDate;
	struct times *currentTime;

	if (clockVars.isBeingSet == 0) {
		drawTextAt(0, 0, "not setting", hspi);
		drawTextAt(0, 10, "     ", hspi);
		getDateTime(currentDate, currentTime);
		sprintf(str, "%2u:%2u:%2u", currentTime->hr, currentTime->min, currentTime->sec);
		drawTextAt(0, 60, str, hspi);
		sprintf(str, "%s, %2u, %4u   %s", monthNames[currentDate->month], currentDate->date, currentDate->yr, weekdayNames[currentDate->weekday]);
		drawTextAt(0, 70, str, hspi);
	}
	else if (clockVars.isBeingSet == 1) {
		drawTextAt(0, 0, "setting... ", hspi);
		switch (clockVars.fieldBeingSet) {
			case 1: drawTextAt(0, 10, "min  ", hspi); break;
			case 2: drawTextAt(0, 10, "hr   ", hspi); break;
			case 3: drawTextAt(0, 10, "year ", hspi); break;
			case 4: drawTextAt(0, 10, "month", hspi); break;
			case 5: drawTextAt(0, 10, "day  ", hspi); break;
			default: break;
		}
		sprintf(str, "%2u:%2u   ", clockVars.timeToSet->hr, clockVars.timeToSet->min);
		drawTextAt(0, 60, str, hspi);
		sprintf(str, "%s, %2u, %4u      ", monthNames[clockVars.dateToSet->month], clockVars.dateToSet->date, clockVars.dateToSet->yr);
		drawTextAt(0, 70, str, hspi);
	}
}

void updateTimerDisplay(SPI_HandleTypeDef *hspi) {
	char str[40];
	uint32_t timerVal;
	uint8_t hr, min, sec;

	if (timerVars.isBeingSet == 0) {
		timerVal = watchTimerSeconds;

		if (timerVal != 0) {
			hr = timerVal / 3600;
			timerVal %= 3600;
			min = timerVal / 60;
			timerVal %= 60;
			sec = timerVal;

			sprintf(str, "%2u:%2u:%2u", hr, min, sec);
			drawTextAt(0, 60, str, hspi);
		}
		else {
			drawTextAt(0, 60, "        ", hspi);
		}

		drawTextAt(0, 10, "     ", hspi);
		if (isTimerRunning == 0) {
			drawTextAt(0, 0, "not running", hspi);
		}
		else if (isTimerRunning == 1) {
			drawTextAt(0, 0, "running    ", hspi);
			sprintf(str, "%2u:%2u:%2u", timerVars.timeToSet->hr, timerVars.timeToSet->min, timerVars.timeToSet->sec);
			drawTextAt(0, 50, str, hspi);
//			sprintf(str, " %lu", watchTimerSeconds);
//			drawTextAt(0, 70, str, hspi);
		}
	}
	else if (timerVars.isBeingSet == 1) {
		drawTextAt(0, 0, "setting... ", hspi);
		switch (timerVars.fieldBeingSet) {
			case 1: drawTextAt(0, 10, "sec  ", hspi); break;
			case 2: drawTextAt(0, 10, "min  ", hspi); break;
			case 3: drawTextAt(0, 10, "hr   ", hspi); break;
			default: break;
		}
		sprintf(str, "%2u:%2u:%2u", timerVars.timeToSet->hr, timerVars.timeToSet->min, timerVars.timeToSet->sec);
		drawTextAt(0, 60, str, hspi);
	}
}

void updateAlarmDisplay(SPI_HandleTypeDef *hspi) {
	char str[40];

	if (alarmVars.isBeingSet == 0) {
		drawTextAt(0, 10, "     ", hspi);
		if (isAlarmRunning == 0) {
			drawTextAt(0, 0, "not running", hspi);
			drawTextAt(0, 60, "              ", hspi);		// clears line used in other cases. probably should wrap in graphics function
		}
		else if (isAlarmRunning == 1) {
			drawTextAt(0, 0, "running    ", hspi);
			sprintf(str, "%2u:%2u:%2u %s", alarmVars.alarmToSet->hr, alarmVars.alarmToSet->min, alarmVars.alarmToSet->sec, weekdayNames[alarmVars.alarmToSet->weekday]);
			drawTextAt(0, 60, str, hspi);
		}
	}
	else if (alarmVars.isBeingSet == 1) {
		drawTextAt(0, 0, "setting... ", hspi);
		switch (alarmVars.fieldBeingSet) {
			case 1: drawTextAt(0, 10, "sec  ", hspi); break;
			case 2: drawTextAt(0, 10, "min  ", hspi); break;
			case 3: drawTextAt(0, 10, "hr   ", hspi); break;
			case 4: drawTextAt(0, 10, "day  ", hspi); break;
			default: break;
		}
		// maybe make this more efficient
		sprintf(str, "%2u:%2u:%2u %s", alarmVars.alarmToSet->hr, alarmVars.alarmToSet->min, alarmVars.alarmToSet->sec, weekdayNames[alarmVars.alarmToSet->weekday]);
		drawTextAt(0, 60, str, hspi);
	}
}

void updateStopwatchDisplay(SPI_HandleTypeDef *hspi) {
	char str[40];
	uint32_t stopwatchVal;
	uint8_t hr, min, sec;

	// translating 32-bit counter to hours, minutes, seconds
	stopwatchVal = stopwatchCNT;

	hr = stopwatchVal / 3600;
	stopwatchVal %= 3600;
	min = stopwatchVal / 60;
	stopwatchVal %= 60;
	sec = stopwatchVal;

	sprintf(str, "%2u:%2u:%2u", hr, min, sec);
	drawTextAt(0, 60, str, hspi);
	if (isStopwatchRunning == 0) {
		drawTextAt(0, 0, "not running", hspi);
	}
	else if (isStopwatchRunning == 1) {
		drawTextAt(0, 0, "running    ", hspi);
	}
}

// calculator for number of days in a month given a month and accounting for leap years
uint8_t maxDaysInMonth(uint8_t month, uint16_t year) {
	if (month == RTC_MONTH_JANUARY ||
		month == RTC_MONTH_MARCH   ||
		month == RTC_MONTH_MAY     ||
		month == RTC_MONTH_JULY    ||
		month == RTC_MONTH_AUGUST  ||
		month == RTC_MONTH_OCTOBER ||
		month == RTC_MONTH_DECEMBER) {
		return 31;
	}
	else if (month == RTC_MONTH_APRIL     ||
			 month == RTC_MONTH_JUNE      ||
			 month == RTC_MONTH_SEPTEMBER ||
			 month == RTC_MONTH_NOVEMBER) {
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

void initFace() {
	isFaceBeingChanged = 1;
	faceOnDisplay = faceClock;
	updateFace.clock = 1;
}

// this sure is a big callback
// need to complete
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == BUTTON1) buttons.is1Pressed = 1;
	if (GPIO_Pin == BUTTON2) buttons.is2Pressed = 1;
	if (GPIO_Pin == BUTTON3) buttons.is3Pressed = 1;
	if (GPIO_Pin == BUTTON4) buttons.is4Pressed = 1;
}
