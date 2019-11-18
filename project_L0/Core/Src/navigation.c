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
				getDateTime(&currentDate, &currentTime, &hrtc);
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

// this sure is a big callback
// need to complete
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	// do i have to initialize it here or does declaration in .h set them all to 0?
	// statics are 0 on start

	/* program flow:
	 *   check current face used
	 *   check current variables and check button pressed
	 */
//	HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);	// should run for any button
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
	if (GPIO_Pin == BUTTON0) {
		face = (face + 1) % NUM_FACES;
		updateFace = 1;
	}
	// use RTC
	if (face == faceClock) {
		updateClock = 1;
		if (GPIO_Pin == BUTTON1 && clockSet) {
			// change fields up, do nothing if not setting clock
			switch (clockField) {
				case 1: tempClockTimes.min = (tempClockTimes.min+1) % 60; break;
				case 2: tempClockTimes.hr = (tempClockTimes.hr+1) % 24; break;
				case 3: tempClockDate.yr++; break;		// supposed to be between large numbers. no need for bounds checking
				case 4: tempClockDate.month = (tempClockDate.month+1) % 12 + 1; break;
				case 5: tempClockDate.date = (tempClockDate.date+1) % 31; break;		// make more robust?
				default: break;
			}
		}
		if (GPIO_Pin == BUTTON2 && clockSet) {
			// change fields down, do nothing if not setting clock
			switch (clockField) {
				case 1: tempClockTimes.min = tempClockTimes.min == 0 ? 59 : tempClockTimes.min-1; break;
				case 2: tempClockTimes.hr = tempClockTimes.hr == 0 ? 24 : tempClockTimes.hr-1; break;
				case 3: tempClockDate.yr--; break;		// supposed to be from 1950-2050. no need to do bounds checking
				case 4: tempClockDate.month = tempClockDate.month == 1 ? 12 : tempClockDate.month-1; break;
				case 5: tempClockDate.date = tempClockDate.date == 0 ? 31 : tempClockDate.date-1; break;
				default: break;
			}
		}
		if (GPIO_Pin == BUTTON3) {
			clockField = (clockField + 1) % (NUM_CLOCKFIELDS + 1);
			if (clockField != 0) {
				clockSet = 1;
				if (clockField == 1) getDateTime(&tempClockDate, &tempClockTimes, &hrtc);	// should pull current time on setting 1st field
			}
			else {
				clockSet = 0;
				// second set to 0, weekday ignored(?)
				setDateTime(&tempClockDate, &tempClockTimes, &hrtc);
			}
		}
		// checks on clock set for other buttons here
	}
	// use lptimer
	else if (face == faceTimer) {
		updateTimer = 1;
		if (timerRunning == 0) {
			if (GPIO_Pin == BUTTON1) {
				if (timerSet == 0) timerRunning = 1;
				else {
					switch (timerField) {
						case 1: tempTimer.sec = (tempTimer.sec+1) % 60; break;
						case 2: tempTimer.min = (tempTimer.min+1) % 60; break;
						case 3: tempTimer.hr = (tempTimer.hr+1) % 24; break;
						default: break;
					}
				}
			}
			if (GPIO_Pin == BUTTON2) {
				if (timerSet == 1) {
					switch (timerField) {
						case 1: tempTimer.sec = tempTimer.sec == 0 ? 59 : tempTimer.sec-1; break;
						case 2: tempTimer.min = tempTimer.min == 0 ? 59 : tempTimer.min-1; break;
						case 3: tempTimer.hr = tempTimer.hr == 0 ? 23 : tempTimer.hr-1; break;
						default: break;
					}
				}
			}
			if (GPIO_Pin == BUTTON3) {
				timerField = (timerField + 1) % (NUM_TIMERFIELDS + 1);
				if (timerField != 0) {
					timerSet = 1;
					if (timerField == 1) {
						tempTimer.sec = 0;
						tempTimer.min = 0;
						tempTimer.hr = 0;
					}
				}
				else {
					timerSet = 0;
					timerRunning = 1;	// careful where this gets set/unset
					setTimer(&tempTimer, &hrtc, &htim21);
				}
			}
		}
		// not sure if this set is in scope of buttons (probably, but not entirely?)
		// complete this
		else if (timerRunning == 1) {
			if (GPIO_Pin == BUTTON1) {
				// start timer hw
				timerRunning = 1;
			}
			if (GPIO_Pin == BUTTON2) {
				// pause timer hw
				timerRunning = 0;
			}
			if (GPIO_Pin == BUTTON3) {
				// stop and clear timer hw
				timerRunning = 0;
			}
		}
	}
	// use RTC alarm
	else if (face == faceAlarm) {
		updateAlarm = 1;
		if (alarmRunning == 0) {
			if (GPIO_Pin == BUTTON1 && alarmSet) {
				// change field up
				switch (alarmField) {
					case 1: tempAlarm.sec = (tempAlarm.sec + 1) % 60; break;
					case 2: tempAlarm.min = (tempAlarm.min + 1) % 60; break;
					case 3: tempAlarm.hr = (tempAlarm.hr + 1) % 24; break;
					case 4: tempAlarm.weekday = (tempAlarm.weekday + 1) % 7 + 1; break;
					default: break;
				}
			}
			if (GPIO_Pin == BUTTON2 && alarmSet) {
				// change field down
				switch (alarmField) {
					case 1: tempAlarm.sec = tempAlarm.sec == 0 ? 59 : tempAlarm.sec-1;
					case 2: tempAlarm.min = tempAlarm.min == 0 ? 59 : tempAlarm.min-1;
					case 3: tempAlarm.hr = tempAlarm.hr == 0 ? 23 : tempAlarm.hr-1;
					case 4: tempAlarm.weekday = tempAlarm.weekday == 1 ? 7 : tempAlarm.weekday-1;
				}
			}
			if (GPIO_Pin == BUTTON3) {
				// toggle between fields
				alarmField = (alarmField + 1) % (NUM_ALARMFIELDS + 1);
				if (alarmField != 0) {
					alarmSet = 1;
					if (alarmField == 1) {
						tempAlarm.sec = 0;
						tempAlarm.min = 0;
						tempAlarm.hr = 0;
						tempAlarm.weekday = 1;
					}
				}
				else {
					alarmSet = 0;
					alarmRunning = 1;
				}
			}
		}
		// not entirely in scope of buttons
		// complete this
		else if (alarmRunning == 1) {
//			if (GPIO_Pin == BUTTON1) {
//				// none
//			}
//			if (GPIO_Pin == BUTTON2) {
//				// none
//			}
			if (GPIO_Pin == BUTTON3) {
				// stop and clear alarm hw
				alarmRunning = 0;
			}
		}
	}
	// use regular timer
	else if (face == faceStopwatch) {
		updateStopwatch = 1;
		if (GPIO_Pin == BUTTON1) {	// start/stop
			if (stopwatchRunning == 0) {
				stopwatchRunning = 1;
				runStopwatch(&hlptim1);
			}
			else {
				stopwatchRunning = 0;
				pauseStopwatch(&hlptim1);
			}
		}
		if (GPIO_Pin == BUTTON2) {
			// pull data and set lap
			lapPrev = lapCurrent;
			lapCurrent = stopwatchCNT;
		}
		if (GPIO_Pin == BUTTON3) {
			// clear stopwatch hw
			stopwatchRunning = 0;
			clearStopwatch(&hlptim1);
		}
	}
}

// callback to test static variable behavior. comment out when not used.
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//	// expected behavior: blue -> green -> red if static variable is set to 0 on declaration
//	// else red -> green -> blue
//
//	// is blue -> green -> red
//	if (clockSet == 0) {
//		fillScreen(ST77XX_BLUE, &hspi1);
//		clockSet = 1;
//	}
//	else if (clockSet == 1) {
//		fillScreen(ST77XX_GREEN, &hspi1);
//		clockSet = 2;
//	}
//	else {
//		fillScreen(ST77XX_RED, &hspi1);
//		clockSet = 0;
//	}
//}
