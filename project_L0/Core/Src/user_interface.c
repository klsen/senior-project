/*
 * File for handling navigation between different apps/interfaces.
 * uses global flags (concurrency problem?).
 */

#include "user_interface.h"

// use static for timer/alarm/stopwatch variables, changeface, and face used.
// needed only in updatedisplay/buttons, but each func shares these variables
static struct clockVariables clockVars = {0};
static struct timerVariables timerVars = {0};
static struct alarmVariables alarmVars = {0};
static struct stopwatchVariables stopwatchVars = {0};
static uint8_t isFaceBeingChanged = 1;
static uint8_t faceOnDisplay = faceClock;

const char* weekdayNames[8] = {
	"",			// padding so i can avoid dealing with out-of-bounds access
	"Monday",	// hal weekday indexing is 1-based
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Sunday"
};

const char* monthNames[13] = {
	"",			// month names also start with 1
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sept",
	"Oct",
	"Nov",
	"Dec"
};

// button interrupt(s)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
	HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
	HAL_NVIC_ClearPendingIRQ(EXTI2_3_IRQn);
	HAL_NVIC_ClearPendingIRQ(EXTI4_15_IRQn);

	if (GPIO_Pin == BUTTON1) buttons.is1Pressed = 1;
	if (GPIO_Pin == BUTTON2) buttons.is2Pressed = 1;
	if (GPIO_Pin == BUTTON3) buttons.is3Pressed = 1;
	if (GPIO_Pin == BUTTON4) buttons.is4Pressed = 1;

	HAL_TIM_Base_Start_IT(&htim6);

	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);		// should run for any button
}

void updateState(RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *motorBacklightTim, TIM_HandleTypeDef *buttonTim, SPI_HandleTypeDef *hspi) {
	/* program flow:
	 *   check current face used
	 *   check current variables and check button pressed
	 */
	// button 1 changes the face on screen.
	if (buttons.is1Pressed) {
		buttons.is1Pressed = 0;
		isFaceBeingChanged = 1;
		faceOnDisplay = (faceOnDisplay + 1) % NUM_FACES;
		switch (faceOnDisplay) {
			case faceClock: updateFace.clock = 1; break;
			case faceTimer: updateFace.timer = 1; break;
			case faceAlarm: updateFace.alarm = 1; break;
			case faceStopwatch: updateFace.stopwatch = 1; break;
			default: break;
		}
	}

	static uint8_t s = 0;
	switch(s) {
		case 0:	if (buttons.is2Pressed) s++; break;
		case 1: if (buttons.is2Pressed) s++; else if (buttons.is1Pressed || buttons.is3Pressed || buttons.is4Pressed) s = 0; break;
		case 2: if (buttons.is3Pressed) s++; else if (buttons.is1Pressed || buttons.is2Pressed || buttons.is4Pressed) s = 0; break;
		case 3: if (buttons.is3Pressed) s++; else if (buttons.is1Pressed || buttons.is2Pressed || buttons.is4Pressed) s = 0; break;
		case 4: if (buttons.is4Pressed) s++; else if (buttons.is1Pressed || buttons.is2Pressed || buttons.is3Pressed) s = 0; break;
		case 5: if (buttons.is4Pressed) {TFT_startup(hspi); s = 0;} break;
		default: break;
	}
	char str[10];
	sprintf(str, "%d", s);
	setTextSize(1);
	drawTextAt(0, 0, str, hspi);

//	if (buttons.is3Pressed) turnDisplayOn(hspi);
//	if (buttons.is4Pressed) turnDisplayOff(hspi);

	if (faceOnDisplay == faceClock) updateClockState(hrtc);
	else if (faceOnDisplay == faceTimer) updateTimerState(timerStopwatchTim, motorBacklightTim);
	else if (faceOnDisplay == faceAlarm) updateAlarmState(hrtc, motorBacklightTim);
	else if (faceOnDisplay == faceStopwatch) updateStopwatchState(timerStopwatchTim);

	// some decisions in respective state functions dont clear these flags
	buttons.is1Pressed = buttons.is2Pressed = buttons.is3Pressed = buttons.is4Pressed = 0;
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
void updateClockState(RTC_HandleTypeDef *hrtc) {
	// change fields up, do nothing if not setting clock
	if (buttons.is2Pressed && clockVars.isBeingSet) {
		buttons.is2Pressed = 0;
		updateFace.clock = 1;
		switch (clockVars.fieldBeingSet) {
			case 1: clockVars.timeToSet->min = (clockVars.timeToSet->min+1) % 60; break;
			case 2: clockVars.timeToSet->hr = (clockVars.timeToSet->hr+1) % 24; break;
			case 3: clockVars.dateToSet->yr++; break;		// supposed to be between large numbers. no need for bounds checking
			case 4: clockVars.dateToSet->month = (clockVars.dateToSet->month) % 12 + 1; break;
			case 5: clockVars.dateToSet->date = ((clockVars.dateToSet->date) % maxDaysInMonth(clockVars.dateToSet->month, clockVars.dateToSet->yr)) + 1; break;
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
			case 4: //clockVars.dateToSet->month = clockVars.dateToSet->month == 1 ? 12 : clockVars.dateToSet->month-1; break;
				if (clockVars.dateToSet->month == 1) clockVars.dateToSet->month = 12;
				else clockVars.dateToSet->month--;
				break;
			case 5:
				if (clockVars.dateToSet->date == 1) clockVars.dateToSet->date = maxDaysInMonth(clockVars.dateToSet->month, clockVars.dateToSet->yr);
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
			if (clockVars.fieldBeingSet == 1) {
				getDateTime(clockVars.dateToSet, clockVars.timeToSet, hrtc);
				HAL_RTC_DeactivateAlarm(hrtc, RTC_ALARM_B);
			}
		}
		else {
			clockVars.isBeingSet = 0;

			// second set to 0, weekday ignored
			setDateTime(clockVars.dateToSet, clockVars.timeToSet, hrtc);
			setClockAlarm(hrtc);
		}
	}
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
void updateTimerState(TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *motorTim) {
	if (timerVars.isBeingSet) {
		if (buttons.is2Pressed) {
			buttons.is2Pressed = 0;
			updateFace.timer = 1;

			// set field up
			switch (timerVars.fieldBeingSet) {
				case 1: timerVars.timeToSet->sec = (timerVars.timeToSet->sec+1) % 60; break;
				case 2: timerVars.timeToSet->min = (timerVars.timeToSet->min+1) % 60; break;
				case 3: timerVars.timeToSet->hr = (timerVars.timeToSet->hr+1) % 100; break;
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
					if (timerVars.timeToSet->hr == 0) timerVars.timeToSet->hr = 99;		// no limit on hour, since we're not using day
					else timerVars.timeToSet->hr--;
					break;
				default: break;
			}
		}
	}
	// set and ready to run
	else if (timerVars.isSet) {
		if (buttons.is2Pressed && isTimerRunning == 0 && timerCounter != 0) {
			buttons.is2Pressed = 0;
			updateFace.timer = 1;

			// start timer
			runTimer(timerStopwatchTim);
			isTimerRunning = 1;
			isTimerPaused = 0;
		}
		if (buttons.is3Pressed && isTimerRunning && timerCounter != 0) {
			buttons.is3Pressed = 0;
			updateFace.timer = 1;

			// pause timer
			pauseTimer(timerStopwatchTim);
			isTimerRunning = 0;
			isTimerPaused = 1;
		}
		if (buttons.is4Pressed) {
			buttons.is4Pressed = 0;
			updateFace.timer = 1;

			// stop and clear timer
			stopTimer(timerStopwatchTim);
			timerVars.isSet = 0;
			isTimerRunning = 0;
			isTimerPaused = 0;
		}
//		if (isTimerDone) {
//			isTimerRunning = 0;
//			isTimerPaused = 0;
//			timerCounter = timeToSeconds(timerVars.timeToSet);
//			runMotor(motorTim);
//		}
	}
	// not done? might be done (other buttons start/stop timer)
	if (buttons.is4Pressed) {
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
		else if (timeToSeconds(timerVars.timeToSet) != 0) {
			timerVars.isBeingSet = 0;
			timerVars.isSet = 1;
			isTimerDone = 0;
			timerCounter = timeToSeconds(timerVars.timeToSet);
		}
		else {
			timerVars.isBeingSet = 0;
			timerVars.isSet = 0;
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
void updateAlarmState(RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *motorTim) {
	if (buttons.is2Pressed && alarmVars.isBeingSet) {
		buttons.is2Pressed = 0;
		updateFace.alarm = 1;

		// change fields up
		switch (alarmVars.fieldBeingSet) {
			case 1: alarmVars.alarmToSet->sec = (alarmVars.alarmToSet->sec + 1) % 60; break;
			case 2: alarmVars.alarmToSet->min = (alarmVars.alarmToSet->min + 1) % 60; break;
			case 3: alarmVars.alarmToSet->hr = (alarmVars.alarmToSet->hr + 1) % 24; break;
			case 4: alarmVars.alarmToSet->weekday = (alarmVars.alarmToSet->weekday) % 7 + 1; break;
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

		if (alarmVars.isSet == 0) {
			// toggle between fields
			alarmVars.fieldBeingSet = (alarmVars.fieldBeingSet + 1) % (NUM_ALARMFIELDS + 1);
			if (alarmVars.fieldBeingSet != 0) {
				alarmVars.isBeingSet = 1;
				if (alarmVars.fieldBeingSet == 1) {
					struct dates d = {0};
					struct times t = {0};
					getDateTime(&d, &t, hrtc);
					alarmVars.alarmToSet->sec = t.sec;
					alarmVars.alarmToSet->min = t.min;
					alarmVars.alarmToSet->hr = t.hr;
					alarmVars.alarmToSet->weekday = d.weekday;
				}
			}
			else {
				alarmVars.isBeingSet = 0;
				alarmVars.isSet = 1;
				setAlarm(alarmVars.alarmToSet, hrtc);
			}
		}
		else {
			// stop and clear alarm hw
			alarmVars.isSet = 0;
			HAL_RTC_DeactivateAlarm(hrtc, RTC_ALARM_A);
		}
	}
//	if (isAlarmDone) {
//		runMotor(motorTim);
//	}
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
void updateStopwatchState(TIM_HandleTypeDef *timerStopwatchTim) {
	if (buttons.is2Pressed) {	// start/stop
		buttons.is2Pressed = 0;
		updateFace.stopwatch = 1;

		if (isStopwatchRunning == 0) {
			runStopwatch(timerStopwatchTim);
			isStopwatchRunning = 1;
			isStopwatchPaused = 0;
		}
		else {
			pauseStopwatch(timerStopwatchTim);
			isStopwatchRunning = 0;
			isStopwatchPaused = 1;
		}
	}
	if (buttons.is3Pressed && stopwatchCounter != 0) {
		buttons.is3Pressed = 0;
		updateFace.stopwatch = 1;

		// pull data and set lap
		stopwatchVars.lapPrev = stopwatchVars.lapCurrent;
		stopwatchVars.lapCurrent = stopwatchCounter;
	}
	if (buttons.is4Pressed) {
		buttons.is4Pressed = 0;
		updateFace.stopwatch = 1;

		// clear stopwatch hw
		clearStopwatch(timerStopwatchTim);
		stopwatchVars.lapCurrent = 0;
		stopwatchVars.lapPrev = 0;
		isStopwatchRunning = 0;
		isStopwatchPaused = 0;
	}
}

// update screen based on global variables
// going in main, so it's executing in a while loop
//   software interrupt on flag so that this doesn't run all the time?
void updateDisplay(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	// change faces
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

		drawButtons(hspi);
	}

	// update clock face
	if (faceOnDisplay == faceClock) {
		if (updateFace.clock == 1) {
			updateFace.clock = 0;
			setBackgroundColor(ST77XX_CYAN);
			updateClockDisplay(hrtc, hspi);
		}
	}
	// update timer face
	else if (faceOnDisplay == faceTimer) {
		if (updateFace.timer == 1) {
			updateFace.timer = 0;
			setBackgroundColor(ST77XX_GREEN);
			updateTimerDisplay(hspi);
		}
	}
	// update alarm face
	else if (faceOnDisplay == faceAlarm) {
		if (updateFace.alarm == 1) {
			updateFace.alarm = 0;
			setBackgroundColor(ST77XX_MAGENTA);
			updateAlarmDisplay(hspi);
		}
	}
	// update stopwatch face
	else if (faceOnDisplay == faceStopwatch) {
		if (updateFace.stopwatch == 1) {
			updateFace.stopwatch = 0;
			setBackgroundColor(ST77XX_YELLOW);
			updateStopwatchDisplay(hspi);
		}
	}

	// is called a lot and redrawn every time. inefficient, but w/e
	drawBattery(battPercentage, hspi);
}

void updateClockDisplay(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	struct dates currentDate = {0};
	struct times currentTime = {0};

	if (clockVars.isBeingSet == 0) {
		getDateTime(&currentDate, &currentTime, hrtc);
		drawClock(&currentDate, &currentTime, hspi);

		setTextSize(1);
		// clear line that says "setting ___"
		clearTextLine(44, hspi);

		// draw button text
		drawButtonText("", "", "set", hspi);
	}
	else if (clockVars.isBeingSet == 1) {
		// draw button text
		if (clockVars.fieldBeingSet == 1) drawButtonText("up", "down", "change", hspi);

		setTextSize(1);
		switch (clockVars.fieldBeingSet) {
			case 1:	drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting minute...", hspi); break;
			case 2:	drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting hour...", hspi);	break;
			case 3: drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting year...", hspi); break;
			case 4: drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting month...", hspi); break;
			case 5: drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting date...", hspi); break;
			default: break;
		}

		drawClock(clockVars.dateToSet, clockVars.timeToSet, hspi);
	}
}

void updateTimerDisplay(SPI_HandleTypeDef *hspi) {
	struct times currentTimer = {0};

	if (timerVars.isBeingSet == 0) {
		if (timerVars.isSet == 0) {
			setTextSize(2);
			clearTextLine(68, hspi);	// clear timer time text

			// write "timer unset"
			setTextSize(1);
			clearTextLine(52, hspi);	// clear setting ___ text
			drawCenteredTextWithPadding(WIDTH/2, 84, 12, "timer unset", hspi);

			// draw button text
			drawButtonText("", "", "set", hspi);
		}
		else if (isTimerDone == 0) {
			secondsToTime(&currentTimer, timerCounter);
			drawTimer(&currentTimer, hspi);

			// write "timer set!" when timer is set, but not running
			setTextSize(1);
			clearTextLine(52, hspi);	// clear setting ___ text
			if (isTimerPaused == 1) {
				drawCenteredTextWithPadding(WIDTH/2, 84, 12, "timer paused", hspi);
			}
			else if (isTimerRunning == 0 && timerCounter != 0) {
				drawCenteredTextWithPadding(WIDTH/2, 84, 12, "timer set!", hspi);
			}
			else {
				clearTextLine(84, hspi);
			}

			// draw button text
			drawButtonText("run", "pause", "clear", hspi);
		}
		else {
			// should occur after running (case above). states should be coded to let you run the same time again
			secondsToTime(&currentTimer, timerCounter);
			drawTimer(&currentTimer, hspi);

			setTextSize(1);
			drawCenteredTextWithPadding(WIDTH/2, 84, 12, "timer done!", hspi);
		}
	}
	else if (timerVars.isBeingSet == 1) {
		// draw button text
		drawButtonText("up", "down", "change", hspi);

		// write filler text
		switch (timerVars.fieldBeingSet) {
			case 1: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting second...", hspi); break;
			case 2: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting minute...", hspi); break;
			case 3: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting hour...", hspi); break;
			default: break;
		}

		drawTimer(timerVars.timeToSet, hspi);
	}
}

void updateAlarmDisplay(SPI_HandleTypeDef *hspi) {
	if (alarmVars.isBeingSet == 0) {
		if (alarmVars.isSet == 0) {
			setTextSize(3);
			clearTextLine(68, hspi);	// clear alarm time text

			setTextSize(1);
			clearTextLine(52, hspi);
			drawCenteredTextWithPadding(WIDTH/2, 100, 11, "alarm unset", hspi);

			// draw button text
			drawButtonText("", "", "set", hspi);
		}
		else if (isAlarmDone == 0) {
			setTextSize(1);
			drawCenteredTextWithPadding(WIDTH/2, 100, 11, "alarm set", hspi);
			drawAlarm(alarmVars.alarmToSet, hspi);

			// draw button text
			drawButtonText("", "", "clear", hspi);
		}
		else {
			// should only run after case above. should let you run alarm again (alarm doesn't get deactivated, so it'll trigger again if you wait a week)
			setTextSize(1);
			drawCenteredTextWithPadding(WIDTH/2, 100, 11, "alarm done!", hspi);
		}
	}
	else if (alarmVars.isBeingSet == 1) {
		setTextSize(1);
		switch (alarmVars.fieldBeingSet) {
			case 1: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting second...", hspi); break;
			case 2: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting minute...", hspi); break;
			case 3: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting hour...", hspi); break;
			case 4: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting day...", hspi); break;
			default: break;
		}

		// draw button text
		drawButtonText("up", "down", "change", hspi);

		drawAlarm(alarmVars.alarmToSet, hspi);
	}
}

void updateStopwatchDisplay(SPI_HandleTypeDef *hspi) {
	drawStopwatch(stopwatchCounter, hspi);
	drawStopwatchLap(stopwatchVars.lapCurrent-stopwatchVars.lapPrev, hspi);

	if (isStopwatchRunning == 0) drawButtonText("run", "lap", "clear", hspi);
	else if (isStopwatchRunning == 1) drawButtonText("pause", "lap", "clear", hspi);
}

// ---- drawing functions related specifically to the user interface ----
void drawButton(uint8_t x_center, uint8_t y_center, SPI_HandleTypeDef *hspi) {
	// bounds checking. probably already done in draw/fillRect
	if (x_center-5 < 0 || x_center+5 > WIDTH || y_center-5 < 0 || y_center+5 > HEIGHT) return;

	// draw rect size 8 with 1 pixel border
	// parameters give center position of graphic
	drawRect(x_center-5, y_center-5, 10, 10, ST77XX_BLACK, hspi);
	fillRect(x_center-4, y_center-4, 8, 8, ST77XX_WHITE, hspi);
}

void drawButtons(SPI_HandleTypeDef *hspi) {
	// 3 buttons. positioned so their text boxes, which are centered over button, can have equal spacing left and right
	drawButton(22, HEIGHT-15, hspi);		// button 1
	drawButton(64, HEIGHT-15, hspi);		// button 2
	drawButton(106, HEIGHT-15, hspi);		// button 3
}

void drawButtonText(const char *str1, const char *str2, const char *str3, SPI_HandleTypeDef *hspi) {
	setTextSize(1);
	drawCenteredTextWithPadding(22, HEIGHT-28, 7, str1, hspi);		// button 1
	drawCenteredTextWithPadding(64, HEIGHT-28, 7, str2, hspi);		// button 2
	drawCenteredTextWithPadding(106, HEIGHT-28, 7, str3, hspi);		// button 3
}

void drawTitle(char *str, SPI_HandleTypeDef *hspi) {
	uint8_t strSize = strlen(str);

	// drawing title
	if (12*strSize < WIDTH) {			// about string size = 10 for width = 128
		setTextSize(2);
		setCursor((WIDTH-12*strSize)/2, 10);
	}
	else if (6*strSize < WIDTH) {		// about string size = 21 for width = 128
		setTextSize(1);
		setCursor((WIDTH-6*strSize)/2, 10);
	}
	else {
		setTextSize(1);
		sprintf(str, "it's too long");		// should not need to worry about null access, since this string is shorter than case above
		setCursor((WIDTH-6*strSize)/2, 10);
	}

	setTextColor(ST77XX_BLACK);
	drawText(str, hspi);
}

void drawBattery(uint8_t batteryLevel, SPI_HandleTypeDef *hspi) {
	// doesn't move and is used on an empty screen, so shouldn't need to clear then print
	char str[5];

	// drawing battery symbol. hard coded to be 6x13, upper left corner on (49,28)
	drawVLine(28, 30, 10, ST77XX_BLACK, hspi);		// left col
	drawVLine(54, 30, 10, ST77XX_BLACK, hspi);		// right col
	drawHLine(50, 41, 4, ST77XX_BLACK, hspi);		// bottom
	drawHLine(50, 29, 4, ST77XX_BLACK, hspi);		// top bottom level
	drawHLine(51, 28, 2, ST77XX_BLACK, hspi);		// top upper level

	uint16_t color = ST77XX_GREEN;
	if (batteryLevel < 20) color = ST77XX_RED;
	fillRect(50, 28+(100-batteryLevel)/10, 4, batteryLevel/10, ST77XX_GREEN, hspi);
	fillRect(50, 28, 4, (100-batteryLevel)/10, ST77XX_WHITE, hspi);

	setTextSize(1);
	if (batteryLevel >= 20) color = ST77XX_BLACK;		// reusing variable for more obfuscated code.
	setTextColor(color);
	sprintf(str, "%3d", batteryLevel);
	drawTextAt(55, 33, str, hspi);
}

// draw time and date
// should optimize to only redraw part that changed
void drawClock(struct dates *d, struct times *t, SPI_HandleTypeDef *hspi) {
	// notes on paper.
	char str[40];

	// no need to draw padding for those that always have the same length
	// drawing hr and min, 12-hr format
	if (t->hr % 12 == 0) sprintf(str, "%2d:%02d", 12, t->min);
	else sprintf(str, "%2d:%02d", t->hr%12, t->min);
	setTextSize(3);
	setTextColor(ST77XX_BLACK);
	drawCenteredText(52, 60, str, hspi);

	// drawing sec
	sprintf(str, "%02d", t->sec);
	setTextSize(2);
	drawCenteredText(109, 68, str, hspi);

	// drawing AM/PM text
	setTextSize(1);
	if (t->hr < 12) drawCenteredText(103, 60, "AM", hspi);
	else drawCenteredText(103, 60, "PM", hspi);

	// drawing date
	setTextSize(1);
	sprintf(str, "%s %d %04d", monthNames[d->month], d->date, d->yr);
	drawCenteredTextWithPadding(WIDTH/2, 84, 11, str, hspi);

	// drawing weekday
	drawCenteredTextWithPadding(WIDTH/2, 92, 9, weekdayNames[d->weekday], hspi);
}

void drawTimer(struct times *t, SPI_HandleTypeDef *hspi) {
	char str[40];

	// only drawing hr:min:sec of timer
	setTextSize(2);
	sprintf(str, "%2d:%2d:%2d", t->hr, t->min, t->sec);
	drawCenteredText(WIDTH/2, HEIGHT/2-12, str, hspi);		// about y=68

	// leaving room to draw "timer set!/unset"
}

void drawAlarm(struct alarmTimes *a, SPI_HandleTypeDef *hspi) {
	char str[40];

	// drawing hr:min:sec
	setTextSize(2);
	sprintf(str, "%2d:%2d:%2d", a->hr, a->min, a->sec);
	drawCenteredText(WIDTH/2, 68, str, hspi);

	// drawing weekday
	setTextSize(1);
	drawCenteredTextWithPadding(WIDTH/2, 84, 9, weekdayNames[a->weekday], hspi);
}

void drawStopwatch(uint32_t seconds, SPI_HandleTypeDef *hspi) {
	struct times t = {0};
	char str[40];

	secondsToTime(&t, seconds);

	// drawing hr:min:sec
	setTextSize(2);
	sprintf(str, "%2d:%2d:%2d", t.hr, t.min, t.sec);
	drawCenteredText(WIDTH/2, 68, str, hspi);

	// leaving room for lap
}

void drawStopwatchLap(uint32_t seconds, SPI_HandleTypeDef *hspi) {
	struct times t = {0};
	char str[40];

	secondsToTime(&t, seconds);

	// drawing hr:min:sec
	setTextSize(1);
	sprintf(str, "lap: %2d:%2d:%2d", t.hr, t.min, t.sec);
	drawCenteredText(WIDTH/2, 84, str, hspi);
}
// ---- end of drawing functions ----

// initializes variables. should be called at the start of the run
void initFace() {
	faceOnDisplay = faceClock;
	updateFace.clock = 1;

	clockVars.dateToSet = (struct dates *)calloc(1, sizeof(struct dates *));
	clockVars.timeToSet = (struct times *)calloc(1, sizeof(struct times *));
	timerVars.timeToSet = (struct times *)calloc(1, sizeof(struct times *));
	alarmVars.alarmToSet = (struct alarmTimes *)calloc(1, sizeof(struct alarmTimes *));
}
