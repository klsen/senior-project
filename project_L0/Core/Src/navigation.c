/*
 * File for handling navigation between different apps/interfaces.
 * uses global flags (concurrency problem?).
 */

#include "navigation.h"

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

	// change these to call functions instead of doing all work here? same for updatedisplay
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
	if (faceOnDisplay == faceClock) {
		// change fields up, do nothing if not setting clock
		if (buttons.is2Pressed && clockVars.isBeingSet) {
			buttons.is2Pressed = 0;
			switch (clockVars.fieldBeingSet) {
				case 1: clockVars.timeToSet.min = (clockVars.timeToSet.min+1) % 60; break;
				case 2: clockVars.timeToSet.hr = (clockVars.timeToSet.hr+1) % 24; break;
				case 3: clockVars.dateToSet.yr++; break;		// supposed to be between large numbers. no need for bounds checking
				case 4: clockVars.dateToSet.month = (clockVars.dateToSet.month+1) % 12 + 1; break;
				case 5: clockVars.dateToSet.date = (clockVars.dateToSet.date+1) % 31; break;		// make more robust?
				default: break;
			}
		}
		// change fields down, do nothing if not setting clock
		if (buttons.is3Pressed && clockVars.isBeingSet) {
			buttons.is3Pressed = 0;
			switch (clockVars.fieldBeingSet) {
				case 1:
					if (clockVars.timeToSet.min == 0) clockVars.timeToSet.min = 59;
					else clockVars.timeToSet.min--;
					break;
				case 2:
					if (clockVars.timeToSet.hr == 0) clockVars.timeToSet.hr = 23;
					else clockVars.timeToSet.hr--;
					break;
				case 3: clockVars.dateToSet.yr--; break;		// supposed to be from 1950-2050. no need to do bounds checking
				case 4: clockVars.dateToSet.month = clockVars.dateToSet.month == 1 ? 12 : clockVars.dateToSet.month-1; break;
					if (clockVars.dateToSet.month == RTC_MONTH_JANUARY) clockVars.dateToSet.month = RTC_MONTH_DECEMBER;
					else clockVars.dateToSet.month--;
				case 5:
					if (clockVars.dateToSet.date == 0) clockVars.dateToSet.date = 31;
					else clockVars.dateToSet.date--;
					break;
				default: break;
			}
		}
		// switches between setting mode and default mode. changes between different clock fields
		if (buttons.is4Pressed) {
			buttons.is4Pressed = 0;
			clockVars.fieldBeingSet = (clockVars.fieldBeingSet + 1) % (NUM_CLOCKFIELDS + 1);
			if (clockVars.fieldBeingSet != 0) {
				clockVars.isBeingSet = 1;

				// should pull current time when first entering setting mode
				if (clockField == 1) getDateTime(&clockVars.dateToSet, &clockVars.timeToSet);
			}
			else {
				clockVars.isBeingSet = 0;

				// second set to 0, weekday ignored
				setDateTime(&clockVars.dateToSet, &clockVars.timeToSet);
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
	else if (faceOnDisplay == faceTimer) {
		if (timerVars.isBeingSet) {
			if (buttons.is2Pressed) {
				buttons.is2Pressed = 0;

				// set field up
				switch (timerVars.fieldBeingSet) {
					case 1: timerVars.timeToSet.sec = (timerVars.timeToSet.sec+1) % 60; break;
					case 2: timerVars.timeToSet.min = (timerVars.timeToSet.min+1) % 60; break;
					case 3: timerVars.timeToSet.hr = (timerVars.timeToSet.hr+1) % 24; break;
					default: break;
				}
			}
			if (buttons.is3Pressed) {
				buttons.is3Pressed = 0;

				// set field down
				switch (timerVars.fieldBeingSet) {
					case 1:
						if (timerVars.timeToSet.sec == 0) timerVars.timeToSet.sec = 59;
						else timerVars.timeToSet.sec--;
						break;
					case 2:
						if (timerVars.timeToSet.min == 0) timerVars.timeToSet.min = 59;
						else timerVars.timeToSet.min--;
						break;
					case 3:
						if (timerVars.timeToSet.hr == 0) timerVars.timeToSet.hr = 23;
						else timerVars.timeToSet.hr--;
						break;
					default: break;
				}
			}
		}

		// not done
		else if (timerVars.isSet) {
			if (buttons.is2Pressed && isTimerRunning == 0) {
				buttons.is2Pressed = 0;
				// start timer
				isTimerRunning = 1;
			}
			if (buttons.is3Pressed && isTimerRunning) {
				buttons.is3Pressed = 0;
				// pause timer
				isTimerRunning = 0;
			}
			if (buttons.is4Pressed) {
				buttons.is4Pressed = 0;

				// stop and clear timer
				timerVars.isSet = 0;
				isTimerRunning = 0;
			}
		}

		// not done? might be done (other buttons start/stop timer)
		else if (buttons.is4Pressed) {
			buttons.is4Pressed = 0;

			// change field/mode
			timerVars.fieldBeingSet = (timerVars.fieldBeingSet + 1) % (NUM_TIMERFIELDS + 1);
			if (timerVars.fieldBeingSet != 0) {
				timerVars.isBeingSet = 1;
				timerVars.isSet = 0;

				// set temp fields to 0 when first entering setting mode
				if (timerVars.fieldBeingSet == 1) {
					timerVars.timeToSet.sec = 0;
					timerVars.timeToSet.min = 0;
					timerVars.timeToSet.hr = 0;
				}
			}
			else {
				timerVars.isBeingSet = 0;
				timerVars.isSet = 1;
				// setTimer(&timerVars.timeToSet);
			}
		}

//		if (isTimerRunning == 0) {
//			if (GPIO_Pin == BUTTON1) {
//				if (timerSet == 0) timerRunning = 1;
//				else {
//					switch (timerField) {
//						case 1: tempTimer.sec = (tempTimer.sec+1) % 60; break;
//						case 2: tempTimer.min = (tempTimer.min+1) % 60; break;
//						case 3: tempTimer.hr = (tempTimer.hr+1) % 24; break;
//						default: break;
//					}
//				}
//			}
//			if (GPIO_Pin == BUTTON2) {
//				if (timerSet == 1) {
//					switch (timerField) {
//						case 1: tempTimer.sec = tempTimer.sec == 0 ? 59 : tempTimer.sec-1; break;
//						case 2: tempTimer.min = tempTimer.min == 0 ? 59 : tempTimer.min-1; break;
//						case 3: tempTimer.hr = tempTimer.hr == 0 ? 23 : tempTimer.hr-1; break;
//						default: break;
//					}
//				}
//			}
//			if (GPIO_Pin == BUTTON3) {
//				timerField = (timerField + 1) % (NUM_TIMERFIELDS + 1);
//				if (timerField != 0) {
//					timerSet = 1;
//					if (timerField == 1) {
//						tempTimer.sec = 0;
//						tempTimer.min = 0;
//						tempTimer.hr = 0;
//					}
//				}
//				else {
//					timerSet = 0;
//					timerRunning = 1;	// careful where this gets set/unset
//					setTimer(&tempTimer);
//				}
//			}
//		}
//		// not sure if this set is in scope of buttons (probably, but not entirely?)
//		// complete this
//		else if (timerRunning == 1) {
//			if (GPIO_Pin == BUTTON1) {
//				// start timer hw
//				timerRunning = 1;
//			}
//			if (GPIO_Pin == BUTTON2) {
//				// pause timer hw
//				timerRunning = 0;
//			}
//			if (GPIO_Pin == BUTTON3) {
//				// stop and clear timer hw
//				timerRunning = 0;
//				stopTimerDisplay();
//				watchTimerSeconds = 0;
//				HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_B);
//			}
//		}
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
	else if (face == faceAlarm) {
		if (buttons.is2Pressed && alarmVars.isBeingSet) {
			buttons.is2Pressed = 0;

			// change fields up
			switch (alarmVars.fieldBeingSet) {
				case 1: alarmVars.alarmToSet.sec = (alarmVars.alarmToSet.sec + 1) % 60; break;
				case 2: alarmVars.alarmToSet.min = (alarmVars.alarmToSet.min + 1) % 60; break;
				case 3: alarmVars.alarmToSet.hr = (alarmVars.alarmToSet.hr + 1) % 24; break;
				case 4: alarmVars.alarmToSet.weekday = (alarmVars.alarmToSet.weekday + 1) % 7 + 1; break;
				default: break;
			}
		}
		if (buttons.is3Pressed && alarmVars.isBeingSet) {
			buttons.is3Pressed = 0;

			// change fields down
			switch (alarmVars.fieldBeingSet) {
				case 1:
					if (alarmVars.alarmToSet.sec == 0) alarmVars.alarmToSet.sec = 59;
					else alarmVars.alarmToSet.sec--;
					break;
				case 2:
					if (alarmVars.alarmToSet.min == 0) alarmVars.alarmToSet.min = 59;
					else alarmVars.alarmToSet.min--;
					break;
				case 3:
					if (alarmVars.alarmToSet.hr == 0) alarmVars.alarmToSet.hr = 23;
					else alarmVars.alarmToSet.hr--;
					break;
				case 4:
					if (alarmVars.alarmToSet.weekday == RTC_WEEKDAY_MONDAY) alarmVars.alarmToSet.weekday = RTC_WEEKDAY_SUNDAY;
					else alarmVars.alarmToSet.weekday--;
					break;
				default: break;
			}
		}
		if (buttons.is4Pressed) {
			buttons.is4Pressed = 0;

			if (isAlarmRunning == 0) {
				// toggle between fields
				alarmVars.fieldBeingSet = (alarmVars.fieldBeingSet + 1) % (NUM_ALARMFIELDS + 1);
				if (alarmVars.fieldBeingSet != 0) {
					alarmVars.isBeingSet = 1;
					if (alarmVars.fieldBeingSet == 1) {
						struct dates d;
						struct times t;
						getDateTime(&d, &t);
						alarmVars.alarmToSet.sec = t.sec;
						alarmVars.alarmToSet.min = t.min;
						alarmVars.alarmToSet.hr = t.hr;
						alarmVars.alarmToSet.weekday = d.weekday;
					}
				}
				else {
					alarmVars.isBeingSet = 0;
					isAlarmRunning = 1;
					setAlarm(&alarmVars.alarmToSet);
				}
			}
			else {
				// stop and clear alarm hw
				isAlarmRunning = 0;
				HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
			}
		}
//		if (alarmRunning == 0) {
//			if (GPIO_Pin == BUTTON1 && alarmSet) {
//				// change field up
//				switch (alarmField) {
//					case 1: tempAlarm.sec = (tempAlarm.sec + 1) % 60; break;
//					case 2: tempAlarm.min = (tempAlarm.min + 1) % 60; break;
//					case 3: tempAlarm.hr = (tempAlarm.hr + 1) % 24; break;
//					case 4: tempAlarm.weekday = (tempAlarm.weekday + 1) % 7 + 1; break;
//					default: break;
//				}
//			}
//			if (GPIO_Pin == BUTTON2 && alarmSet) {
//				// change field down
//				switch (alarmField) {
//					case 1: tempAlarm.sec = tempAlarm.sec == 0 ? 59 : tempAlarm.sec-1;
//					case 2: tempAlarm.min = tempAlarm.min == 0 ? 59 : tempAlarm.min-1;
//					case 3: tempAlarm.hr = tempAlarm.hr == 0 ? 23 : tempAlarm.hr-1;
//					case 4: tempAlarm.weekday = tempAlarm.weekday == 1 ? 7 : tempAlarm.weekday-1;
//				}
//			}
//			if (GPIO_Pin == BUTTON3) {
//				// toggle between fields
//				alarmField = (alarmField + 1) % (NUM_ALARMFIELDS + 1);
//				if (alarmField != 0) {
//					alarmSet = 1;
//					if (alarmField == 1) {
//						struct dates d;
//						struct times t;
//						getDateTime(&d, &t);
//						tempAlarm.sec = t.sec;
//						tempAlarm.min = t.min;
//						tempAlarm.hr = t.hr;
//						tempAlarm.weekday = d.weekday;
//					}
//				}
//				else {
//					alarmSet = 0;
//					alarmRunning = 1;
//					setAlarm(&tempAlarm);
//				}
//			}
//		}
//		// not entirely in scope of buttons
//		// complete this
//		else if (alarmRunning == 1) {
////			if (GPIO_Pin == BUTTON1) {
////				// none
////			}
////			if (GPIO_Pin == BUTTON2) {
////				// none
////			}
//			if (GPIO_Pin == BUTTON3) {
//				// stop and clear alarm hw
//				alarmRunning = 0;
//				HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
//			}
//		}
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
	else if (face == faceStopwatch) {
		if (buttons.is2Pressed) {	// start/stop
			buttons.is2Pressed = 0;

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

			// pull data and set lap
			stopwatchVars.lapPrev = stopwatchVars.lapCurrent;
			stopwatchVars.lapCurrent = stopwatchCNT;		// did this variable get changed to something else?
		}
		if (buttons.is4Pressed) {
			buttons.is4Pressed = 0;

			// clear stopwatch hw
			isStopwatchRunning = 0;
			clearStopwatch();
		}
	}
}

// update screen based on global variables
// going in main, so it's executing in a while loop
//   software interrupt on flag so that this doesn't run all the time?
void updateDisplay(SPI_HandleTypeDef *hspi) {
	char str[40];
	uint32_t stopwatchVal, timerVal;
	uint8_t hr, min, sec;
	struct dates currentDate;
	struct times currentTime;
	// update main clock face
	// missing space for current time
	if (updateFace == 1) {
		updateFace = 0;
		if (face == faceClock) {
			clearScreen(ST77XX_CYAN, hspi);
			drawTextAt(0, HEIGHT-10, "main     ", hspi);
		}
		if (face == faceTimer) {
			clearScreen(ST77XX_GREEN, hspi);
			drawTextAt(0, HEIGHT-10, "timer    ", hspi);
		}
		if (face == faceAlarm) {
			clearScreen(ST77XX_MAGENTA, hspi);
			drawTextAt(0, HEIGHT-10, "alarm    ", hspi);
		}
		if (face == faceStopwatch) {
			clearScreen(ST77XX_YELLOW, hspi);
			drawTextAt(0, HEIGHT-10, "stopwatch", hspi);
		}
	}
	if (face == faceClock) {
		if (updateClock == 1) {
			updateClock = 0;
			if (clockSet == 0) {
				drawTextAt(0, 0, "not setting", hspi);
				drawTextAt(0, 10, "     ", hspi);
				getDateTime(&currentDate, &currentTime);
				sprintf(str, "%2u:%2u:%2u", currentTime.hr, currentTime.min, currentTime.sec);
				drawTextAt(0, 60, str, hspi);
				sprintf(str, "%s, %2u, %4u   %s", monthNames[currentDate.month], currentDate.date, currentDate.yr, weekdayNames[currentDate.weekday]);
				drawTextAt(0, 70, str, hspi);
			}
			else if (clockSet == 1) {
				drawTextAt(0, 0, "setting... ", hspi);
				switch (clockField) {
					case 1: drawTextAt(0, 10, "min  ", hspi); break;
					case 2: drawTextAt(0, 10, "hr   ", hspi); break;
					case 3: drawTextAt(0, 10, "year ", hspi); break;
					case 4: drawTextAt(0, 10, "month", hspi); break;
					case 5: drawTextAt(0, 10, "day  ", hspi); break;
					default: break;
				}
				sprintf(str, "%2u:%2u   ", tempClockTimes.hr, tempClockTimes.min);
				drawTextAt(0, 60, str, hspi);
				sprintf(str, "%s, %2u, %4u      ", monthNames[tempClockDate.month], tempClockDate.date, tempClockDate.yr);
				drawTextAt(0, 70, str, hspi);
			}
		}
	}
	// update timer face
	else if (face == faceTimer) {
		if (updateTimer == 1) {
			updateTimer = 0;
			if (timerSet == 0) {
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
				if (timerRunning == 0) {
					drawTextAt(0, 0, "not running", hspi);
				}
				else if (timerRunning == 1) {
					drawTextAt(0, 0, "running    ", hspi);
					sprintf(str, "%2u:%2u:%2u", tempTimer.hr, tempTimer.min, tempTimer.sec);
					drawTextAt(0, 50, str, hspi);
//					sprintf(str, " %lu", watchTimerSeconds);
//					drawTextAt(0, 70, str, hspi);
				}
			}
			else if (timerSet == 1) {
				drawTextAt(0, 0, "setting... ", hspi);
				switch (timerField) {
					case 1: drawTextAt(0, 10, "sec  ", hspi); break;
					case 2: drawTextAt(0, 10, "min  ", hspi); break;
					case 3: drawTextAt(0, 10, "hr   ", hspi); break;
					default: break;
				}
				sprintf(str, "%2u:%2u:%2u", tempTimer.hr, tempTimer.min, tempTimer.sec);
				drawTextAt(0, 60, str, hspi);
			}
		}
	}
	// update alarm face
	else if (face == faceAlarm) {
		if (updateAlarm == 1) {
			updateAlarm = 0;
			if (alarmSet == 0) {
				drawTextAt(0, 10, "     ", hspi);
				if (alarmRunning == 0) {
					drawTextAt(0, 0, "not running", hspi);
					drawTextAt(0, 60, "              ", hspi);		// clears line used in other cases. probably should wrap in graphics function
				}
				else if (alarmRunning == 1) {
					drawTextAt(0, 0, "running    ", hspi);
					sprintf(str, "%2u:%2u:%2u %s", watchAlarm.hr, watchAlarm.min, watchAlarm.sec, weekdayNames[watchAlarm.weekday]);
					drawTextAt(0, 60, str, hspi);
				}
			}
			else if (alarmSet == 1) {
				drawTextAt(0, 0, "setting... ", hspi);
				switch (alarmField) {
					case 1: drawTextAt(0, 10, "sec  ", hspi); break;
					case 2: drawTextAt(0, 10, "min  ", hspi); break;
					case 3: drawTextAt(0, 10, "hr   ", hspi); break;
					case 4: drawTextAt(0, 10, "day  ", hspi); break;
					default: break;
				}
				// maybe make this more efficient
				sprintf(str, "%2u:%2u:%2u %s", tempAlarm.hr, tempAlarm.min, tempAlarm.sec, weekdayNames[tempAlarm.weekday]);
				drawTextAt(0, 60, str, hspi);
			}
		}
	}
	// update stopwatch face
	else if (face == faceStopwatch) {
		if (updateStopwatch == 1) {
			updateStopwatch = 0;
			// translating 32-bit counter to hours, minutes, seconds
			stopwatchVal = stopwatchCNT;

			hr = stopwatchVal / 3600;
			stopwatchVal %= 3600;
			min = stopwatchVal / 60;
			stopwatchVal %= 60;
			sec = stopwatchVal;

			sprintf(str, "%2u:%2u:%2u", hr, min, sec);
			drawTextAt(0, 60, str, hspi);
			if (stopwatchRunning == 0) {
				drawTextAt(0, 0, "not running", hspi);
			}
			else if (stopwatchRunning == 1) {
				drawTextAt(0, 0, "running    ", hspi);
			}
		}
	}
}


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

// this sure is a big callback
// need to complete
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == BUTTON1) buttonPressed.is1Pressed = 1;
	if (GPIO_Pin == BUTTON2) buttonPressed.is2Pressed = 1;
	if (GPIO_Pin == BUTTON3) buttonPressed.is3Pressed = 1;
	if (GPIO_Pin == BUTTON4) buttonPressed.is4Pressed = 1;
}
