/*
 * file for handling navigation between different apps/interfaces.
 * uses global flags (concurrency problem).
 */

#include "navigation.h"

// update screen based on global variables
// going in main, so it's executing in a while loop
//   software interrupt on flag so that this doesn't run all the time?
void updateDisplay(SPI_HandleTypeDef *hspi) {
//	faceChange = 1;			// writes every time it enters function call (makes flag effectively always 1)
	// update main clock face
	// missing space for current time
	if (face == faceMain) {
		if (faceChange == 1) {
			fillScreen(ST77XX_CYAN, hspi);
			drawText(0, HEIGHT-10, 1, ST77XX_BLACK, "main     ", hspi);
			faceChange = 0;
		}
		if (clockSet == 0) drawText(0, 0, 1, ST77XX_BLACK, "set       ", hspi);
		else if (clockSet == 1) {
			drawText(0, 0, 1, ST77XX_BLACK, "setting...", hspi);
			switch (clockField) {
				case 1: drawText(0, 10, 1, ST77XX_BLACK, "min  ", hspi); break;
				case 2: drawText(0, 10, 1, ST77XX_BLACK, "hr   ", hspi); break;
				case 3: drawText(0, 10, 1, ST77XX_BLACK, "year ", hspi); break;
				case 4: drawText(0, 10, 1, ST77XX_BLACK, "month", hspi); break;
				case 5: drawText(0, 10, 1, ST77XX_BLACK, "day  ", hspi); break;
				default: break;
			}
		}
	}
	// update timer face
	else if (face == faceTimer) {
		if (faceChange == 1) {
			fillScreen(ST77XX_GREEN, hspi);
			drawText(0, HEIGHT-10, 1, ST77XX_BLACK, "timer    ", hspi);
			faceChange = 0;
		}
		if (timerSet == 0) {
			if (timerRunning == 0) drawText(0, 0, 1, ST77XX_BLACK, "not running", hspi);
			else if (timerRunning == 1) drawText(0, 0, 1, ST77XX_BLACK, "running    ", hspi);
		}
		else if (timerSet == 1) {
			drawText(0, 0, 1, ST77XX_BLACK, "setting...", hspi);
			switch (timerField) {
				case 1: drawText(0, 10, 1, ST77XX_BLACK, "sec  ", hspi); break;
				case 2: drawText(0, 10, 1, ST77XX_BLACK, "min  ", hspi); break;
				case 3: drawText(0, 10, 1, ST77XX_BLACK, "hr   ", hspi); break;
				default: break;
			}
		}
	}
	// update alarm face
	else if (face == faceAlarm) {
		if (faceChange == 1) {
			fillScreen(ST77XX_MAGENTA, hspi);
			drawText(0, HEIGHT-10, 1, ST77XX_BLACK, "alarm    ", hspi);
			faceChange = 0;
		}
		if (alarmSet == 0) {
			if (alarmRunning == 0) drawText(0, 0, 1, ST77XX_BLACK, "not running", hspi);
			else if (alarmRunning == 1) drawText(0, 0, 1, ST77XX_BLACK, "running    ", hspi);
		}
		else if (alarmSet == 1) {
			drawText(0, 0, 1, ST77XX_BLACK, "setting...", hspi);
			switch (alarmField) {
				case 1: drawText(0, 10, 1, ST77XX_BLACK, "sec  ", hspi); break;
				case 2: drawText(0, 10, 1, ST77XX_BLACK, "min  ", hspi); break;
				case 3: drawText(0, 10, 1, ST77XX_BLACK, "hr   ", hspi); break;
				case 4: drawText(0, 10, 1, ST77XX_BLACK, "day  ", hspi); break;
				default: break;
			}
		}
	}
	// update stopwatch face
	else if (face == faceStopwatch) {
		if (faceChange == 1) {
			fillScreen(ST77XX_YELLOW, hspi);
			drawText(0, HEIGHT-10, 1, ST77XX_BLACK, "stopwatch", hspi);
			faceChange = 0;
		}
		if (stopwatchRunning == 0) drawText(0, 0, 1, ST77XX_BLACK, "not running", hspi);
		else if (stopwatchRunning == 1) drawText(0, 0, 1, ST77XX_BLACK, "running    ", hspi);
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
				if (alarmField != 0) alarmSet = 1;
				else {
					alarmSet = 0;
					alarmRunning = 1;
				}
			}
		}
		// not entirely in scope of buttons
		else if (alarmRunning == 1) {
			if (GPIO_Pin == BUTTON1) {
				// none
			}
			if (GPIO_Pin == BUTTON2) {
				// none
			}
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
