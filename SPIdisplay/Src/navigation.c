/*
 * file for handling navigation between different apps/interfaces.
 * uses global flags (concurrency problem).
 */

#include "TFT_display.h"
#include "main.h"

// enum for different faces used (clock, timer, alarm, stopwatch)
typedef enum {
	faceMain,
	faceTimer,
	faceAlarm,
	faceStopwatch
} displayFaces;

// functions


void updateFace() {

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	static int state = 0;
	HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);

	if (state == 0) {
		fillScreen(ST77XX_BLACK, &hspi1);
		state = 1;
	}
	else if (state == 1) {
		fillScreen(ST77XX_WHITE, &hspi1);
		state = 0;
	}
}

// button hardware interrupt handler

// lay this fucking shit out.
// what functions do i need, what flags do i need, etc.
