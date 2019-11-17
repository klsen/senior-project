/*
 * File for handling navigation between different apps/interfaces.
 * uses global flags (concurrency problem?).
 */

#include "navigation.h"
#include "clocks.h"
#include "timers.h"

// update screen based on global variables
// going in main, so it's executing in a while loop
//   software interrupt on flag so that this doesn't run all the time?
void updateDisplay(SPI_HandleTypeDef *hspi) {
	char* str[40];
	uint32_t stopwatchVal, hr, min, sec;
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
			clearScreen(ST77XX_MAGNETA, hspi);
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
				sprintf(str, "%2d:%2d:%2d", currentTime.hr, currentTime.min, currentTime.sec);
				drawTextAt(0, 60, str, hspi);
				sprintf(str, "%s, %d, %d   %s", monthNames[currentDate.month], currentDate.day, currentDate.yr, weekdayNames[currentDate.weekday]);
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
				sprintf(str, "%2d:%2d   ", tempClockTimes.hr, tempClockTimes.min);
				drawTextAt(0, 60, str, hspi);
				sprintf(str, "%s, %d, %d", monthNames[tempClockDate.month], tempClockDate.day, tempClockDate.yr);
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

					sprintf(str, "%2d:%2d:%2d", hr, min, sec);
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
				sprintf(str, "%2d:%2d:%2d", tempTimer.hr, tempTimer.min, tempTimer.sec);
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
					sprintf(str, "%2d:%2d:%2d %s", watchAlarm.hr, watchAlarm.min, watchAlarm.sec, weekdayNames[watchAlarm.weekday]);
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
				sprintf(str, "%2d:%2d:%2d %s", tempAlarm.hr, tempAlarm.min, tempAlarm.sec, weekdayNames[tempAlarm.weekday]);
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

			sprintf(str, "%2d:%2d:%2d", hr, min, sec);
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
		faceChange = 1;
	}
	// use RTC
	if (face == faceMain) {
		if (GPIO_Pin == BUTTON1 && clockSet) {
			// change fields up, do nothing if not setting clock
		}
		if (GPIO_Pin == BUTTON2 && clockSet) {
			// change fields down, do nothing if not setting clock
		}
		if (GPIO_Pin == BUTTON3) {
			clockField = (clockField + 1) % (NUM_CLOCKFIELDS + 1);
			if (clockField != 0) clockSet = 1;
			else clockSet = 0;
		}
		// checks on clock set for other buttons here
	}
	// use lptimer
	else if (face == faceTimer) {
		if (timerRunning == 0) {
			if (GPIO_Pin == BUTTON1) {
				if (timerSet == 0) timerRunning = 1;
			}
			if (GPIO_Pin == BUTTON2) {
			}
			if (GPIO_Pin == BUTTON3) {
				timerField = (timerField + 1) % (NUM_TIMERFIELDS + 1);
				if (timerField != 0) timerSet = 1;
				else {
					timerSet = 0;
					timerRunning = 1;	// careful where this gets set/unset
				}
			}
		}
		// not sure if this set is in scope of buttons (probably, but not entirely?)
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
		if (alarmRunning == 0) {
			if (GPIO_Pin == BUTTON1) {
				// change field up
			}
			if (GPIO_Pin == BUTTON2) {
				// change field down
			}
			if (GPIO_Pin == BUTTON3) {
				// toggle between fields
				alarmField = (alarmField + 1) % (NUM_ALARMFIELDS + 1);
				if (alarmField != 0) {
					alarmSet = 1;
				}
				else {
					alarmSet = 0;
					alarmRunning = 1;
				}
			}
		}
		// not entirely in scope of buttons
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
		if (GPIO_Pin == BUTTON1) {	// start/stop
			if (stopwatchRunning == 0) stopwatchRunning = 1;
			else stopwatchRunning = 0;
		}
		if (GPIO_Pin == BUTTON2) {
			// pull data and set lap
		}
		if (GPIO_Pin == BUTTON3) {
			// clear stopwatch hw
			stopwatchRunning = 0;
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
