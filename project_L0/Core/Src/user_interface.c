// Implementation file for user_interface.h

#include "user_interface.h"

// each function in this file shares these variables, but these are not used in other files.
static struct clockVariables clockVars = {0};
static struct timerVariables timerVars = {0};
static struct alarmVariables alarmVars = {0};
static struct stopwatchVariables stopwatchVars = {0};
static uint8_t isFaceBeingChanged = 1;
static uint8_t faceOnDisplay = faceClock;

// default values for orientation 0 (portrait)
static uint16_t buttonHSpacing;
static uint16_t buttonVSpacing;
static struct coords button1Coords;		// centered x because i am confused

// externs
extern TIM_HandleTypeDef htim3;

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
	"",			// month values also start with 1
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

// callback for button interrupts.
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	// toggles LED whenever a button is pressed
//	HAL_GPIO_TogglePin(LED3_PORT, LED3_PIN);
//	HAL_GPIO_WritePin(LED3_PORT, LED3_PIN, GPIO_PIN_RESET);

	// disables interrupts for software debouncing
//	HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
//	HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
//	HAL_NVIC_ClearPendingIRQ(EXTI2_3_IRQn);
//	HAL_NVIC_ClearPendingIRQ(EXTI4_15_IRQn);

	// updates flags
	if (GPIO_Pin == BUTTON1) buttons.is1Pressed = 1;
	if (GPIO_Pin == BUTTON2) buttons.is2Pressed = 1;
	if (GPIO_Pin == BUTTON3) buttons.is3Pressed = 1;
	if (GPIO_Pin == BUTTON4) buttons.is4Pressed = 1;

	// runs timer for software debouncing delay
//	HAL_TIM_Base_Start_IT(&htim6);
}

//
void updateState(RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *motorBacklightTim, TIM_HandleTypeDef *buttonTim, SPI_HandleTypeDef *hspi) {
	if (buttons.is1Pressed || buttons.is2Pressed || buttons.is3Pressed || buttons.is4Pressed) {
		// button 1 changes the face on screen.
		if (buttons.is1Pressed) {
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

		// button combo: press 2 and 3 alternatively 5 times to reinit display.
		// needed since screen often turns white when its power supply is rustled, and there's no way to show the information
		static uint8_t s = 0;
		switch(s) {
			case 0:	if (buttons.is2Pressed) s++; break;
			case 1: if (buttons.is3Pressed) s++; else if (buttons.is1Pressed || buttons.is2Pressed || buttons.is4Pressed) s = 0; break;
			case 2: if (buttons.is2Pressed) s++; else if (buttons.is1Pressed || buttons.is3Pressed || buttons.is4Pressed) s = 0; break;
			case 3: if (buttons.is3Pressed) s++; else if (buttons.is1Pressed || buttons.is2Pressed || buttons.is4Pressed) s = 0; break;
			case 4: if (buttons.is2Pressed) s++; else if (buttons.is1Pressed || buttons.is3Pressed || buttons.is4Pressed) s = 0; break;
			case 5: if (buttons.is3Pressed) s++; else if (buttons.is1Pressed || buttons.is2Pressed || buttons.is4Pressed) s = 0; break;
			case 6: if (buttons.is2Pressed) s++; else if (buttons.is1Pressed || buttons.is3Pressed || buttons.is4Pressed) s = 0; break;
			case 7: if (buttons.is3Pressed) s++; else if (buttons.is1Pressed || buttons.is2Pressed || buttons.is4Pressed) s = 0; break;
			case 8: if (buttons.is2Pressed) s++; else if (buttons.is1Pressed || buttons.is3Pressed || buttons.is4Pressed) s = 0; break;
			case 9: if (buttons.is3Pressed) {s = 0; TFT_startup(hspi);} break;
			default: break;
		}

		static uint8_t o = 0;
		if (buttons.is3Pressed) {
			o = getDisplayOrientation() + 1;
			setDisplayOrientation(o, hspi);
			isFaceBeingChanged = 1;
		}

		// run helper functions when their face is on screen
		if (faceOnDisplay == faceClock) updateClockState(hrtc);
		else if (faceOnDisplay == faceTimer) updateTimerState(timerStopwatchTim, motorBacklightTim);
		else if (faceOnDisplay == faceAlarm) updateAlarmState(hrtc, motorBacklightTim);
		else if (faceOnDisplay == faceStopwatch) updateStopwatchState(timerStopwatchTim);

		// flags cleared only when state code has finished executing once
		buttons.is1Pressed = buttons.is2Pressed = buttons.is3Pressed = buttons.is4Pressed = 0;
	}
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
 */
void updateClockState(RTC_HandleTypeDef *hrtc) {
	// check button pressed -> perform action
	static int8_t brightness = 50;
	if (buttons.is2Pressed) {
		updateFace.clock = 1;
		if (clockVars.isBeingSet) {
			switch (clockVars.fieldBeingSet) {
				case 1: clockVars.timeToSet->min = (clockVars.timeToSet->min+1) % 60; break;
				case 2: clockVars.timeToSet->hr = (clockVars.timeToSet->hr+1) % 24; break;
				case 3: clockVars.dateToSet->yr = (clockVars.dateToSet->yr + 1) % 10000; break;		// fit in 4 characters
				case 4: clockVars.dateToSet->month = (clockVars.dateToSet->month) % 12 + 1; break;
				case 5: clockVars.dateToSet->date = ((clockVars.dateToSet->date) % maxDaysInMonth(clockVars.dateToSet->month, clockVars.dateToSet->yr)) + 1; break;
				default: break;
			}
		}
		else {
//			if (brightness < 100) brightness += 10;
			if (brightness == 50) brightness = 10;
			else if (brightness == 10) brightness = 0;
			else brightness = 50;
			setDisplayBacklight(brightness, &htim3);
//			sleepMode();
		}
	}
	// change fields down, do nothing if not setting clock
	if (buttons.is3Pressed) {
		updateFace.clock = 1;
		if (clockVars.isBeingSet) {
			switch (clockVars.fieldBeingSet) {
				case 1:
					if (clockVars.timeToSet->min == 0) clockVars.timeToSet->min = 59;
					else clockVars.timeToSet->min--;
					break;
				case 2:
					if (clockVars.timeToSet->hr == 0) clockVars.timeToSet->hr = 23;
					else clockVars.timeToSet->hr--;
					break;
				case 3: if (clockVars.dateToSet->yr != 0) clockVars.dateToSet->yr--; break;		// limit to positive numbers. no wrap-around
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
		else {
//			if (brightness > 0) brightness -= 10;
//			setDisplayBacklight(brightness, &htim3);
//			stopMode();
//			setDisplayBacklight(brightness, &htim3);
		}
	}
	// switches between setting mode and default mode. changes between different clock fields
	if (buttons.is4Pressed) {
		updateFace.clock = 1;
		clockVars.fieldBeingSet = (clockVars.fieldBeingSet + 1) % (NUM_CLOCKFIELDS + 1);
		if (clockVars.fieldBeingSet != 0) {
			clockVars.isBeingSet = 1;

			// should pull current time when first entering setting mode
			if (clockVars.fieldBeingSet == 1) {
				getDateTime(clockVars.dateToSet, clockVars.timeToSet, hrtc);
				HAL_RTC_DeactivateAlarm(hrtc, RTC_ALARM_B);
			}

			if (clockVars.dateToSet->date > maxDaysInMonth(clockVars.dateToSet->month, clockVars.dateToSet->yr)) {
				clockVars.dateToSet->date = maxDaysInMonth(clockVars.dateToSet->month, clockVars.dateToSet->yr);
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
 */
void updateTimerState(TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *motorTim) {
	// check which button is pressed -> perform action
	if (buttons.is2Pressed) {
		updateFace.timer = 1;
		if (timerVars.isBeingSet) {
			// set field up
			switch (timerVars.fieldBeingSet) {
				case 1: timerVars.timeToSet->sec = (timerVars.timeToSet->sec+1) % 60; break;
				case 2: timerVars.timeToSet->min = (timerVars.timeToSet->min+1) % 60; break;
				case 3: timerVars.timeToSet->hr = (timerVars.timeToSet->hr+1) % 100; break;
				default: break;
			}
		}
		else if (timerVars.isSet && isTimerRunning == 0 && timerCounter != 0) {
			// start timer
			runTimer(timerStopwatchTim);
			isTimerRunning = 1;
			isTimerPaused = 0;
		}
	}
	else if (buttons.is3Pressed) {
		updateFace.timer = 1;
		if (timerVars.isBeingSet) {
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
		else if (timerVars.isSet && isTimerRunning && timerCounter != 0) {
			// pause timer
			pauseTimer(timerStopwatchTim);
			isTimerRunning = 0;
			isTimerPaused = 1;
		}
	}
	else if (buttons.is4Pressed) {
		updateFace.timer = 1;
		if (timerVars.isSet) {
			// stop and clear timer
			stopTimer(timerStopwatchTim);
			timerVars.isSet = 0;
			isTimerRunning = 0;
			isTimerPaused = 0;
		}
		else {
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
}

/*
 * does various things when alarm is the face on screen.
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
 */
void updateAlarmState(RTC_HandleTypeDef *hrtc, TIM_HandleTypeDef *motorTim) {
	// check button pressed -> perform action
	if (buttons.is2Pressed && alarmVars.isBeingSet) {
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
 */
void updateStopwatchState(TIM_HandleTypeDef *timerStopwatchTim) {
	// start/stop
	if (buttons.is2Pressed) {
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
		updateFace.stopwatch = 1;

		// pull data and set lap
		stopwatchVars.lapPrev = stopwatchVars.lapCurrent;
		stopwatchVars.lapCurrent = stopwatchCounter;
	}
	if (buttons.is4Pressed) {
		updateFace.stopwatch = 1;

		// clear stopwatch hw
		clearStopwatch(timerStopwatchTim);
		stopwatchVars.lapCurrent = 0;
		stopwatchVars.lapPrev = 0;
		isStopwatchRunning = 0;
		isStopwatchPaused = 0;
	}
}

// primary function for making changes to display
void updateDisplay(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	// TODO: redraw efficiency improvements
	if (updateFace.clock || updateFace.timer || updateFace.alarm || updateFace.stopwatch || isFaceBeingChanged) {
		if (getDisplayOrientation() % 2 == 0) {		// portrait
			buttonHSpacing = WIDTH/2;
			buttonVSpacing = 24;
			button1Coords.x = WIDTH/4;
			button1Coords.y = HEIGHT-(15+buttonVSpacing);
		}
		else {
			buttonHSpacing = WIDTH/4;
			buttonVSpacing = 0;
			button1Coords.x = WIDTH/8;
			button1Coords.y = HEIGHT - 15;
		}

		switch (faceOnDisplay) {
			case faceClock: drawClockApp(hrtc, hspi); break;
			case faceTimer: drawTimerApp(hrtc, hspi); break;
			case faceAlarm: drawAlarmApp(hrtc, hspi); break;
			case faceStopwatch: drawStopwatchApp(hrtc, hspi); break;
			default: break;
		}

		updateFace.clock = updateFace.timer = updateFace.alarm = updateFace.stopwatch = 0;
	}
	// change faces
//	if (isFaceBeingChanged == 1) {
//		isFaceBeingChanged = 0;
//
//		// drawing titles and boxes that won't be rewritten during normal operation within
//		// a specific face (titles and buttons)
//		if (faceOnDisplay == faceClock) {
//			clearScreen(ST77XX_CYAN, hspi);
//			drawTitle("clock", hspi);
//		}
//		if (faceOnDisplay == faceTimer) {
//			clearScreen(ST77XX_GREEN, hspi);
//			drawTinyTime(hrtc, hspi);
//			drawTitle("timer", hspi);
//		}
//		if (faceOnDisplay == faceAlarm) {
//			clearScreen(ST77XX_MAGENTA, hspi);
//			drawTinyTime(hrtc, hspi);
//			drawTitle("alarm", hspi);
//		}
//		if (faceOnDisplay == faceStopwatch) {
//			clearScreen(ST77XX_YELLOW, hspi);
//			drawTinyTime(hrtc, hspi);
//			drawTitle("stopwatch", hspi);
//		}
//
//		drawBattery(battPercentage, hspi);
//		drawButtons(hspi);
//	}
//
//
//	if (updateFace.clock || updateFace.timer || updateFace.alarm || updateFace.stopwatch) {
//		// update clock face
//		if (faceOnDisplay == faceClock) {
//			if (updateFace.clock == 1) {
//				setBackgroundColor(ST77XX_CYAN);
//				updateClockDisplay(hrtc, hspi);
//			}
//		}
//		// update timer face
//		else if (faceOnDisplay == faceTimer) {
//			if (updateFace.timer == 1) {
//				setBackgroundColor(ST77XX_GREEN);
//				updateTimerDisplay(hspi);
//			}
//			if (updateFace.clock == 1) drawTinyTime(hrtc, hspi);
//		}
//		// update alarm face
//		else if (faceOnDisplay == faceAlarm) {
//			if (updateFace.alarm == 1) {
//				setBackgroundColor(ST77XX_MAGENTA);
//				updateAlarmDisplay(hspi);
//			}
//			if (updateFace.clock == 1) drawTinyTime(hrtc, hspi);
//		}
//		// update stopwatch face
//		else if (faceOnDisplay == faceStopwatch) {
//			if (updateFace.stopwatch == 1) {
//				setBackgroundColor(ST77XX_YELLOW);
//				updateStopwatchDisplay(hspi);
//			}
//			if (updateFace.clock == 1) drawTinyTime(hrtc, hspi);
//		}
//
//		updateFace.clock = updateFace.timer = updateFace.alarm = updateFace.stopwatch = 0;
//	}
}

// helper function for drawing all elements for clock display
void drawClockApp(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	struct dates currentDate = {0};
	struct times currentTime = {0};
	// TODO: checks for orientation
	struct coords modeTextCoords = {WIDTH/2, 44};
	if (getDisplayOrientation() % 2 == 0) modeTextCoords.y = 44;
	else modeTextCoords.y = 32;
	struct coords timeTextCoords = {centeredToLeft(WIDTH/2, 114), modeTextCoords.y+fontH*2};

	// code for full display refresh. should be all portions that aren't rewritten
	if (isFaceBeingChanged) {
		clearScreen(ST77XX_CYAN, hspi);
		drawTitle("clock", hspi);
		drawButtons(hspi);
		drawButtonText(1, "timer", hspi);
		isFaceBeingChanged = 0;
	}

	// different code for different modes
//	drawBattery(WIDTH/2-(4*fontW+6)/2, 5, hspi);		// battery in the middle
	drawTopBar(hrtc, hspi);
	if (clockVars.isBeingSet == 0) {
		drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "", hspi);
		getDateTime(&currentDate, &currentTime, hrtc);
		drawDateTime(timeTextCoords.x, timeTextCoords.y, &currentDate, &currentTime, hspi);
		drawButtonText(2, "", hspi);
		drawButtonText(3, "", hspi);
		drawButtonText(4, "set", hspi);
//		getDateTime(&currentDate, &currentTime, hrtc);
//		drawClock(&currentDate, &currentTime, hspi);
//
//		setTextSize(1);
//		// clear line that says "setting ___"
//		clearTextLine(44, hspi);
//
//		// draw button text
//		drawButtonText("", "", "set", hspi);
	}
	else if (clockVars.isBeingSet == 1) {
		drawDateTime(timeTextCoords.x, timeTextCoords.y, clockVars.dateToSet, clockVars.timeToSet, hspi);

		drawButtonText(2, "up", hspi);
		drawButtonText(3, "down", hspi);
		drawButtonText(4, "next", hspi);

		switch (clockVars.fieldBeingSet) {
			case 1: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting minute...", hspi); break;
			case 2: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting hour...", hspi); break;
			case 3: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting year...", hspi); break;
			case 4: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting month...", hspi); break;
			case 5: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting date...", hspi); break;
			default: break;
		}
//		// draw button text
//		if (clockVars.fieldBeingSet == 1) drawButtonText("up", "down", "change", hspi);
//
//		setTextSize(1);
//		switch (clockVars.fieldBeingSet) {
//			case 1:	drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting minute...", hspi); break;
//			case 2:	drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting hour...", hspi);	break;
//			case 3: drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting year...", hspi); break;
//			case 4: drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting month...", hspi); break;
//			case 5: drawCenteredTextWithPadding(WIDTH/2, 44, 17, "setting date...", hspi); break;
//			default: break;
//		}
//
//		drawClock(clockVars.dateToSet, clockVars.timeToSet, hspi);
	}
}

// helper function for drawing all elements for timer display
void drawTimerApp(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	struct times currentTimer = {0};
	struct coords modeTextCoords = {WIDTH/2, 52};
	if (getDisplayOrientation() % 2 == 0) modeTextCoords.y = 52;
	else modeTextCoords.y = 48;
	struct coords timeTextCoords = {centeredToLeft(WIDTH/2, 96), modeTextCoords.y+fontH*2};


	if (isFaceBeingChanged) {
		clearScreen(ST77XX_GREEN, hspi);
		drawTitle("timer", hspi);
		drawButtons(hspi);
		drawButtonText(1, "alarm", hspi);
		isFaceBeingChanged = 0;
	}

	drawTopBar(hrtc, hspi);
	if (timerVars.isBeingSet == 0) {
		if (timerVars.isSet == 0) {
			drawButtonText(2, "", hspi);
			drawButtonText(3, "", hspi);
			drawButtonText(4, "set", hspi);

			drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "timer unset", hspi);
			fillRect(timeTextCoords.x, timeTextCoords.y, 96, 16, getBackgroundColor(), hspi);
		}
		else if (isTimerDone == 0) {
			drawButtonText(2, "run", hspi);
			drawButtonText(3, "pause", hspi);
			drawButtonText(4, "clear", hspi);
			secondsToTime(&currentTimer, timerCounter);
			drawBasicTime(timeTextCoords.x, timeTextCoords.y, &currentTimer, hspi);

			// write "timer set!" when timer is set, but not running
			if (isTimerPaused == 1) drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "timer paused", hspi);
			else if (isTimerRunning == 0 && timerCounter != 0) drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "timer set!", hspi);
			else drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "", hspi);
		}
		else {
			// should occur after running (case above). TODO: states should be coded to let you run the same time again
			secondsToTime(&currentTimer, timerCounter);
			drawBasicTime(timeTextCoords.x, timeTextCoords.y, &currentTimer, hspi);
			drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "timer done!", hspi);
		}
	}
	else if (timerVars.isBeingSet == 1) {
		drawButtonText(2, "up", hspi);
		drawButtonText(3, "down", hspi);
		drawButtonText(4, "next", hspi);

		switch (timerVars.fieldBeingSet) {
			case 1: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting second...", hspi); break;
			case 2: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting minute...", hspi); break;
			case 3: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting hour...", hspi); break;
			default: break;
		}

		drawBasicTime(timeTextCoords.x, timeTextCoords.y, timerVars.timeToSet, hspi);
//		// draw button text
//		drawButtonText("up", "down", "change", hspi);
//
//		// write filler text
//		switch (timerVars.fieldBeingSet) {
//			case 1: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting second...", hspi); break;
//			case 2: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting minute...", hspi); break;
//			case 3: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting hour...", hspi); break;
//			default: break;
//		}
//
//		drawTimer(timerVars.timeToSet, hspi);
	}
}

// helper function for drawing all elements for alarm display
void drawAlarmApp(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	struct coords modeTextCoords = {WIDTH/2, 48};
	if (getDisplayOrientation() % 2 == 0) modeTextCoords.y = 48;
	else modeTextCoords.y = 36;
	struct coords timeTextCoords = {centeredToLeft(WIDTH/2, 114), modeTextCoords.y+fontH*2};
	struct times alarmTime = {0};


	if (isFaceBeingChanged) {
		clearScreen(ST77XX_MAGENTA, hspi);
		drawTitle("alarm", hspi);
		drawButtons(hspi);
		if (getDisplayOrientation() % 2 == 0) drawButtonText(1, "stopwatch", hspi);
		else drawButtonText(1, "stopw.", hspi);
		isFaceBeingChanged = 0;
	}

	drawTopBar(hrtc, hspi);
	if (alarmVars.isBeingSet == 0) {
		drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "", hspi);
//		setTextSize(1);
//		clearTextLine(52, hspi);	// clear "setting..." text
//		clearTextLine(60, hspi);	// clear am/pm text
		if (alarmVars.isSet == 0) {
			drawButtonText(2, "", hspi);
			drawButtonText(3, "", hspi);
			drawButtonText(4, "set", hspi);
			drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "alarm unset", hspi);
			fillRect(timeTextCoords.x, timeTextCoords.y, 114, 32, getBackgroundColor(), hspi);
//			setTextSize(3);
//			clearTextLine(68, hspi);	// clear alarm time text
//
//			setTextSize(1);
//			drawCenteredTextWithPadding(WIDTH/2, 100, 11, "alarm unset", hspi);
//
//			// draw button text
//			drawButtonText("", "", "set", hspi);
		}
		else if (isAlarmDone == 0) {
			drawButtonText(2, "", hspi);
			drawButtonText(3, "", hspi);
			drawButtonText(4, "clear", hspi);
			drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "alarm set!", hspi);

			alarmTime.hr  = alarmVars.alarmToSet->hr;
			alarmTime.min = alarmVars.alarmToSet->min;
			alarmTime.sec = alarmVars.alarmToSet->sec;
			drawWeekdayTime(timeTextCoords.x, timeTextCoords.y, alarmVars.alarmToSet->weekday, &alarmTime, hspi);
//			setTextSize(1);
//			drawCenteredTextWithPadding(WIDTH/2, 100, 11, "alarm set", hspi);
//			drawAlarm(alarmVars.alarmToSet, hspi);
//
//			// draw button text
//			drawButtonText("", "", "clear", hspi);
		}
		else {
			drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "alarming!", hspi);
			// should only run after case above. TODO: maybe should let you run alarm again (alarm doesn't get deactivated, so it'll trigger again if you wait a week)
//			setTextSize(1);
//			drawCenteredTextWithPadding(WIDTH/2, 100, 11, "alarm done!", hspi);
		}
	}
	else if (alarmVars.isBeingSet == 1) {
		switch (alarmVars.fieldBeingSet) {
			case 1: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting second...", hspi); break;
			case 2: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting minute...", hspi); break;
			case 3: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting hour...", hspi); break;
			case 4: drawCenteredModeText(modeTextCoords.x, modeTextCoords.y, "setting day...", hspi); break;
			default: break;
		}

		drawButtonText(2, "up", hspi);
		drawButtonText(3, "down", hspi);
		drawButtonText(4, "next", hspi);

		alarmTime.hr  = alarmVars.alarmToSet->hr;
		alarmTime.min = alarmVars.alarmToSet->min;
		alarmTime.sec = alarmVars.alarmToSet->sec;
		drawWeekdayTime(timeTextCoords.x, timeTextCoords.y, alarmVars.alarmToSet->weekday, &alarmTime, hspi);
//		setTextSize(1);
//		switch (alarmVars.fieldBeingSet) {
//			case 1: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting second...", hspi); break;
//			case 2: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting minute...", hspi); break;
//			case 3: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting hour...", hspi); break;
//			case 4: drawCenteredTextWithPadding(WIDTH/2, 52, 17, "setting day...", hspi); break;
//			default: break;
//		}
//
//		// draw button text
//		drawButtonText("up", "down", "change", hspi);
//
//		// draw alarm
//		drawAlarm(alarmVars.alarmToSet, hspi);
	}
}

// helper function for drawing all elements for stopwatch display
void drawStopwatchApp(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	struct coords timeTextCoords = {centeredToLeft(WIDTH/2, 96), 68};
	if (getDisplayOrientation() % 2 == 0) timeTextCoords.y = 68;
	else timeTextCoords.y = 52;
	struct coords lapTextCoords = {WIDTH/2, timeTextCoords.y+fontH*2};
	struct times t = {0};
	char str[24];

	if (isFaceBeingChanged) {
		clearScreen(ST77XX_YELLOW, hspi);
		drawTitle("stopwatch", hspi);
		drawButtons(hspi);
		drawButtonText(1, "clock", hspi);
		drawButtonText(3, "lap", hspi);
		drawButtonText(4, "clear", hspi);
		isFaceBeingChanged = 0;
	}

	if (getDisplayOrientation() % 2 == 0) drawTopBar(hrtc, hspi);
	else drawBattery(centeredToLeft(WIDTH/2, 6+4*fontW), 23, hspi);
	secondsToTime(&t, stopwatchCounter);
	drawBasicTime(timeTextCoords.x, timeTextCoords.y, &t, hspi);

	secondsToTime(&t, stopwatchVars.lapCurrent-stopwatchVars.lapPrev);		// TODO: some sort of check?
	setTextSize(1);
	setTextColor(ST77XX_BLACK);
	sprintf(str, "lap: %2d:%2d:%2d", t.hr, t.min, t.sec);
	drawCenteredText(lapTextCoords.x, lapTextCoords.y, str, hspi);

	if (isStopwatchRunning == 0) drawButtonText(2, "run", hspi);
	else drawButtonText(2, "pause", hspi);

//	setTextColor(ST77XX_BLACK);
//	drawStopwatch(stopwatchCounter, hspi);
//	drawStopwatchLap(stopwatchVars.lapCurrent-stopwatchVars.lapPrev, hspi);
//
//	if (isStopwatchRunning == 0) drawButtonText("run", "lap", "clear", hspi);
//	else if (isStopwatchRunning == 1) drawButtonText("pause", "lap", "clear", hspi);
}

// ---- drawing functions related specifically to the user interface ----
// draws a 10x10 box representing a button onto the screen
// uses upper left coords
void drawButton(uint8_t x_center, uint8_t y_center, SPI_HandleTypeDef *hspi) {
	// bounds checking. probably already done in draw/fillRect
	if (x_center-5 < 0 || x_center+5 > WIDTH || y_center-5 < 0 || y_center+5 > HEIGHT) return;		// TODO: fix bounds checking

	// draw rect size 8 with 1 pixel border
	// parameters give center position of graphic
	drawRect(x_center, y_center, 10, 10, ST77XX_BLACK, hspi);
	fillRect(x_center+1, y_center+1, 8, 8, ST77XX_WHITE, hspi);
}

// draws 4 buttons to represent important ui buttons and tell the user their action
void drawButtons(SPI_HandleTypeDef *hspi) {
//	uint16_t buttonHSpacing = WIDTH/2;
//	uint16_t buttonYSpacing = 24;
//	struct coords b1 = {centeredToLeft(button1Coords.x, 10), button1Coords.y};
//	struct coords b2 = {b1.x+buttonHSpacing, b1.y};
//	struct coords b3 = {b1.x, b1.y+buttonVSpacing};
//	struct coords b4 = {b2.x, b3.y};
	struct coords b1 = {0, 0};
	struct coords b2 = {0, 0};
	struct coords b3 = {0, 0};
	struct coords b4 = {0, 0};

	if (getDisplayOrientation() % 2 == 0) {
		b1.x = centeredToLeft(button1Coords.x, 10);
		b1.y = button1Coords.y;
		b2.x = b1.x+buttonHSpacing;
		b2.y = b1.y;
		b3.x = b1.x;
		b3.y = b1.y+buttonVSpacing;
		b4.x = b2.x;
		b4.y = b3.y;
	}
	else {
		b1.x = centeredToLeft(button1Coords.x, 10);
		b1.y = button1Coords.y;
		b2.x = b1.x+buttonHSpacing;
		b2.y = b1.y;
		b3.x = b2.x+buttonHSpacing;
		b3.y = b1.y;
		b4.x = b3.x+buttonHSpacing;
		b4.y = b3.y;
	}

	drawButton(b1.x, b1.y, hspi);
	drawButton(b2.x, b2.y, hspi);
	drawButton(b3.x, b3.y, hspi);
	drawButton(b4.x, b4.y, hspi);
}

// draws text that goes a few pixels over the button
void drawButtonText(uint8_t buttonNo, const char *str, SPI_HandleTypeDef *hspi) {
	// using centered x, not upper left x
	// TODO: make variables file wide. modified in their own function or something
//	uint16_t buttonHSpacing = WIDTH/2;
//	uint16_t buttonYSpacing = 24;							// use more variables? (button height, spacing, fontsize)
	uint8_t maxTextLength = buttonHSpacing/fontW;			// TODO: can you combine variables with nearby funcs?

//	struct coords b1 = {button1Coords.x, button1Coords.y-fontH};
//	struct coords b2 = {b1.x+buttonHSpacing, b1.y};
//	struct coords b3 = {b1.x, b1.y+buttonVSpacing};
//	struct coords b4 = {b2.x, b3.y};

	struct coords b1 = {0, 0};
	struct coords b2 = {0, 0};
	struct coords b3 = {0, 0};
	struct coords b4 = {0, 0};

	if (getDisplayOrientation() % 2 == 0) {
		b1.x = button1Coords.x;
		b1.y = button1Coords.y-fontH;
		b2.x = b1.x+buttonHSpacing;
		b2.y = b1.y;
		b3.x = b1.x;
		b3.y = b1.y+buttonVSpacing;
		b4.x = b2.x;
		b4.y = b3.y;
	}
	else {
		b1.x = button1Coords.x;
		b1.y = button1Coords.y-fontH;
		b2.x = b1.x+buttonHSpacing;
		b2.y = b1.y;
		b3.x = b2.x+buttonHSpacing;
		b3.y = b1.y;
		b4.x = b3.x+buttonHSpacing;
		b4.y = b3.y;
	}

	setTextSize(1);
	setTextColor(ST77XX_BLACK);
	// 1-based, not 0-based
	switch(buttonNo) {
		case 1: drawCenteredTextWithPadding(b1.x, b1.y, maxTextLength, str, hspi); break;
		case 2: drawCenteredTextWithPadding(b2.x, b2.y, maxTextLength, str, hspi); break;
		case 3: drawCenteredTextWithPadding(b3.x, b3.y, maxTextLength, str, hspi); break;
		case 4: drawCenteredTextWithPadding(b4.x, b4.y, maxTextLength, str, hspi); break;
		default: break;
	}
}

void drawButtonTexts(const char *str1, const char *str2, const char *str3, const char *str4, SPI_HandleTypeDef *hspi) {
	// using centered x, not upper left x
//	uint16_t buttonHSpacing = WIDTH/2;
//	uint16_t buttonYSpacing = 24;
	uint8_t maxTextLength = buttonHSpacing/fontW;

//	struct coords b1 = {button1Coords.x, button1Coords.y-fontH};
//	struct coords b2 = {b1.x+buttonHSpacing, b1.y};
//	struct coords b3 = {b1.x, b1.y+buttonVSpacing};
//	struct coords b4 = {b2.x, b3.y};

	struct coords b1 = {0, 0};
	struct coords b2 = {0, 0};
	struct coords b3 = {0, 0};
	struct coords b4 = {0, 0};

	if (getDisplayOrientation() % 2 == 0) {
		b1.x = button1Coords.x;
		b1.y = button1Coords.y-fontH;
		b2.x = b1.x+buttonHSpacing;
		b2.y = b1.y;
		b3.x = b1.x;
		b3.y = b1.y+buttonVSpacing;
		b4.x = b2.x;
		b4.y = b3.y;
	}
	else {
		b1.x = button1Coords.x;
		b1.y = button1Coords.y-fontH;
		b2.x = b1.x+buttonHSpacing;
		b2.y = b1.y;
		b3.x = b2.x+buttonHSpacing;
		b3.y = b1.y;
		b4.x = b3.x+buttonHSpacing;
		b4.y = b3.y;
	}

	setTextSize(1);
	setTextColor(ST77XX_BLACK);
	drawCenteredTextWithPadding(b1.x, b1.y, maxTextLength, str1, hspi);
	drawCenteredTextWithPadding(b2.x, b2.y, maxTextLength, str2, hspi);
	drawCenteredTextWithPadding(b3.x, b3.y, maxTextLength, str3, hspi);
	drawCenteredTextWithPadding(b4.x, b4.y, maxTextLength, str4, hspi);
}

// draws big text on top of the display
void drawTitle(char *str, SPI_HandleTypeDef *hspi) {
	uint8_t strSize = strlen(str);
	uint16_t titleY = 20;

	if (getDisplayOrientation() % 2 == 0) titleY = 20;
	else titleY = 5;

	// drawing title
	// bounds checking
	// TODO: use fontsize variable
	if (12*strSize < WIDTH) {			// about string size = 10 for width = 128
		setTextSize(2);
	}
	else if (6*strSize < WIDTH) {		// about string size = 21 for width = 128
		setTextSize(1);
	}
	else {
		setTextSize(1);
		sprintf(str, "it's too long");		// should not need to worry about null access, since this string is shorter than case above
	}

	setTextColor(ST77XX_BLACK);
	drawCenteredText(WIDTH/2, titleY, str, hspi);
}

// shows when you're setting something or whatever
// use centered x
void drawModeText(uint16_t x, uint16_t y, const char *str, SPI_HandleTypeDef *hspi) {
	setTextSize(1);
	setTextColor(ST77XX_BLACK);
	uint8_t maxLength = WIDTH/fontW;
	drawTextWithPadding(x, y, maxLength, str, hspi);
}

void drawCenteredModeText(uint16_t x, uint16_t y, const char *str, SPI_HandleTypeDef *hspi) {
	setTextSize(1);
	setTextColor(ST77XX_BLACK);
	uint8_t maxLength = WIDTH/fontW;
	drawCenteredTextWithPadding(x, y, maxLength, str, hspi);
}

// draws a battery graphic to represent current battery level
void drawBattery(uint16_t x, uint16_t y, SPI_HandleTypeDef *hspi) {
	// doesn't move and is used on an empty screen, so shouldn't need to clear then print
	char str[5];

	// TODO: draw only when battery level changes
	// drawing battery symbol. hard coded to be 6x13, upper right corner
	drawVLine(x, y+2, 10, ST77XX_BLACK, hspi);		// left col
	drawVLine(x+5, y+2, 10, ST77XX_BLACK, hspi);	// right col
	drawHLine(x+1, y+12, 4, ST77XX_BLACK, hspi);	// bottom
	drawHLine(x+1, y+1, 4, ST77XX_BLACK, hspi);		// top bottom level
	drawHLine(x+2, y, 2, ST77XX_BLACK, hspi);		// top upper level

	// start filling in green/red box depending on battery level
	uint16_t color = ST77XX_GREEN;
	uint8_t batteryLevel = battPercentage;
	if (batteryLevel < 20) color = ST77XX_RED;
	fillRect(x+1, (y+2)+(100-batteryLevel)/10, 4, (batteryLevel+9)/10, color, hspi);	// +9 to avoid having to use float and round()
	fillRect(x+1, y+2, 4, (100-batteryLevel)/10, ST77XX_WHITE, hspi);

	// draw numerical text
	setTextSize(1);
	if (batteryLevel >= 20) color = ST77XX_BLACK;		// reusing variable for more obfuscated code.
	setTextColor(color);
	sprintf(str, "%3d%%", batteryLevel);
	drawTextAt(x+6, y+4, str, hspi);
}

// drawing current time on top of screen when other faces are displayed
void drawTinyTime(uint16_t x, uint16_t y, RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	char str[10];
	struct times currentTime = {0};
	getTime(&currentTime, hrtc);

	if (currentTime.hr % 12 == 0) sprintf(str, "%2d:%02d", 12, currentTime.min);
	else sprintf(str, "%2d:%02d", currentTime.hr%12, currentTime.min);
	setTextSize(1);
	setTextColor(ST77XX_BLACK);
	drawTextAt(x, y, str, hspi);

	if (currentTime.hr < 12) drawTextAt(x+fontW*5, y, "AM", hspi);
	else drawTextAt(x+fontW*5, y, "PM", hspi);
}

// groups the small clock and battery into a horizontal bar on top of the screen
void drawTopBar(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi) {
	uint16_t barY = 5;
	struct coords battCoords = {WIDTH-(6+fontW*4)-5, barY};		// TODO: use fontsize variable
	struct coords clockCoords = {5, barY+4};

	// TODO: if checks to determine whether to redraw or not
	drawTinyTime(clockCoords.x, clockCoords.y, hrtc, hspi);
	drawBattery(battCoords.x, battCoords.y, hspi);
}

// 114x24. draws time only
// use upper left coords pls ty
// originally on (7, 60)
void drawTime(uint16_t x, uint16_t y, struct times *t, SPI_HandleTypeDef *hspi) {
	// notes on paper.
	char str[24];

	// no need to draw padding for those that always have the same length
	// drawing hr and min, 12-hr format
	if (t->hr % 12 == 0) sprintf(str, "%2d:%02d", 12, t->min);
	else sprintf(str, "%2d:%02d", t->hr%12, t->min);
	setTextSize(3);
	setTextColor(ST77XX_BLACK);
	drawTextAt(x, y, str, hspi);

	// drawing sec
	sprintf(str, "%02d", t->sec);
	setTextSize(2);
	drawTextAt(x+3*fontW*5, y+fontH, str, hspi);

	// drawing AM/PM text
	setTextSize(1);
	if (t->hr < 12) drawTextAt(x+3*fontW*5, y, "AM", hspi);
	else drawTextAt(x+3*fontW*5, y, "PM", hspi);

//	// drawing date
//	setTextSize(1);
//	sprintf(str, "%s %d %04d", monthNames[d->month], d->date, d->yr);
//	drawCenteredTextWithPadding(WIDTH/2, 84, 12, str, hspi);
//
//	// drawing weekday
//	drawCenteredTextWithPadding(WIDTH/2, 92, 9, weekdayNames[d->weekday], hspi);
}

// draws both time and date (as it says right there in the function name)
// uses upper left coords
void drawDateTime(uint16_t x, uint16_t y, struct dates *d, struct times *t, SPI_HandleTypeDef *hspi) {
	char str[24];
	uint16_t xc = leftToCentered(x, 114);

	drawTime(x, y, t, hspi);

	// drawing date
	setTextSize(1);
	sprintf(str, "%s %d %04d", monthNames[d->month], d->date, d->yr);
	drawCenteredTextWithPadding(xc, y+24, 12, str, hspi);

	// drawing weekday
	drawCenteredTextWithPadding(xc, y+32, 9, weekdayNames[d->weekday], hspi);
}

// used for alarms, but like...what if...you dont use it for alarms
// uses upper left coords.
// callee's responsibility to unpack alarm struct ty
void drawWeekdayTime(uint16_t x, uint16_t y, uint8_t weekday, struct times *t, SPI_HandleTypeDef *hspi) {
	drawTime(x, y, t, hspi);

	setTextSize(1);
	drawCenteredTextWithPadding(leftToCentered(x, 114), y+24, 9, weekdayNames[weekday], hspi);
}

// only 1-line hr:min:sec
// 96x16
// upper left coords (use helpers if you want to center uwu)
// originally on (16, 68)
void drawBasicTime(uint16_t x, uint16_t y, struct times *t, SPI_HandleTypeDef *hspi) {
	char str[24];
	setTextSize(2);
	setTextColor(ST77XX_BLACK);
	sprintf(str, "%2d:%2d:%2d", t->hr, t->min, t->sec);
	drawTextAt(x, y, str, hspi);
}

//// drawing timer on screen
//void drawTimer(struct times *t, SPI_HandleTypeDef *hspi) {
//	char str[40];
//
//	// only drawing hr:min:sec of timer
//	setTextSize(2);
//	setTextColor(ST77XX_BLACK);
//	sprintf(str, "%2d:%2d:%2d", t->hr, t->min, t->sec);
//	drawCenteredText(WIDTH/2, HEIGHT/2-12, str, hspi);		// about y=68
//
//	// leaving room to draw "timer set!/unset" text
//}
//
//// drawing alarm on screen
//void drawAlarm(struct alarmTimes *a, SPI_HandleTypeDef *hspi) {
//	char str[40];
//
//	// drawing hr:min:sec
//	setTextSize(2);
//	setTextColor(ST77XX_BLACK);
//	if (a->hr % 12 == 0) sprintf(str, "%2d:%2d:%2d", 12, a->min, a->sec);
//	else sprintf(str, "%2d:%2d:%2d", a->hr%12, a->min, a->sec);
//	drawCenteredText(WIDTH/2, 68, str, hspi);
//
//	setTextSize(1);
//	if (a->hr < 12) drawCenteredText(100, 60, "AM", hspi);
//	else drawCenteredText(100, 60, "PM", hspi);
//
//
//	// drawing weekday
//	setTextSize(1);
//	drawCenteredTextWithPadding(WIDTH/2, 84, 9, weekdayNames[a->weekday], hspi);
//}
//
//// drawing stopwatch on screen
//void drawStopwatch(uint32_t seconds, SPI_HandleTypeDef *hspi) {
//	struct times t = {0};
//	char str[40];
//
//	secondsToTime(&t, seconds);
//
//	// drawing hr:min:sec
//	setTextSize(2);
//	setTextColor(ST77XX_BLACK);
//	sprintf(str, "%2d:%2d:%2d", t.hr, t.min, t.sec);
//	drawCenteredText(WIDTH/2, 68, str, hspi);
//
//	// leaving room for lap text
//}
//
//// drawing lap text
//void drawStopwatchLap(uint32_t seconds, SPI_HandleTypeDef *hspi) {
//	struct times t = {0};
//	char str[40];
//
//	secondsToTime(&t, seconds);		// converting
//
//	// drawing hr:min:sec
//	setTextSize(1);
//	setTextColor(ST77XX_BLACK);
//	sprintf(str, "lap: %2d:%2d:%2d", t.hr, t.min, t.sec);
//	drawCenteredText(WIDTH/2, 84, str, hspi);
//}
// ---- end of drawing functions ----

// initializes variables. should be called at the start of program
void initFace() {
	faceOnDisplay = faceClock;
	updateFace.clock = 1;

	// initializing pointers
	clockVars.dateToSet = (struct dates *)calloc(1, sizeof(struct dates *));
	clockVars.timeToSet = (struct times *)calloc(1, sizeof(struct times *));
	timerVars.timeToSet = (struct times *)calloc(1, sizeof(struct times *));
	alarmVars.alarmToSet = (struct alarmTimes *)calloc(1, sizeof(struct alarmTimes *));
}
