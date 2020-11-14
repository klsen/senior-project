/*
 * Shows off available user inputs using the screen.
 * It should have button tap and button hold for all 4 buttons.
 *
 * It's meant to be run in place of the standard while loop.
 *
 * Kenneth Seneres
 * Nov. 13, 2020
 */

#include "input_test.h"

// ui crap
// upper left based coords
static uint16_t bgColor = 0x9710;		// pale green
static uint16_t hSpacing;
static uint16_t vSpacing;
static struct coords b1Box;
static struct coords b2Box;
static struct coords b3Box;
static struct coords b4Box;
static uint8_t b1TapS, b1HoldS, b2TapS, b2HoldS, b3TapS, b3HoldS, b4TapS, b4HoldS;

void input_test(SPI_HandleTypeDef *hspi) {
	// this really fucking hurts my eyes
	hSpacing = getDisplayWidth()/2.5;
	vSpacing = getDisplayHeight()/4;
	b2Box.x = getDisplayWidth()/2;
	b2Box.y = getDisplayHeight()/4;
	b1Box.x = b2Box.x-hSpacing;
	b1Box.y = b2Box.y;
	b3Box.x = b1Box.x;
	b3Box.y = b1Box.y+vSpacing;
	b4Box.x = b1Box.x+hSpacing;
	b4Box.y = b1Box.y+vSpacing;
//	static uint8_t b1TapS, b1HoldS, b2TapS, b2HoldS, b3TapS, b3HoldS, b4TapS, b4HoldS;

	layout(hspi);

	while (1) {
		// add code about buttons here
		// TODO: code about pulling detected inputs
//		switch(input) {
//			case b1Tap:
//				buttonTappedVisual(b1Box.x, b1Box.y, 1, hspi);
//				break;
//			case b1Hold:
//				buttonHeldVisual(b1Box.x, b1Box.y, 1, hspi);
//				break;
//			case b2Tap:
//				buttonTappedVisual(b2Box.x, b2Box.y, 1, hspi);
//				break;
//			case b2Hold:
//				buttonHeldVisual(b2Box.x, b2Box.y, 1, hspi);
//				break;
//			case b3Tap:
//				buttonTappedVisual(b3Box.x, b3Box.y, 1, hspi);
//				break;
//			case b3Hold:
//				buttonHeldVisual(b3Box.x, b3Box.y, 1, hspi);
//				break;
//			case b4Tap:
//				buttonTappedVisual(b4Box.x, b4Box.y, 1, hspi);
//				break;
//			case b4Hold:
//				buttonHeldVisual(b4Box.x, b4Box.y, 1, hspi);
//				break;
//			default:
//				buttonTappedVisual(b1Box.x, b1Box.y, 0, hspi);
//				buttonTappedVisual(b2Box.x, b2Box.y, 0, hspi);
//				buttonTappedVisual(b3Box.x, b3Box.y, 0, hspi);
//				buttonTappedVisual(b4Box.x, b4Box.y, 0, hspi);
//				buttonHeldVisual(b1Box.x, b1Box.y, 0, hspi);
//				buttonHeldVisual(b2Box.x, b2Box.y, 0, hspi);
//				buttonHeldVisual(b3Box.x, b3Box.y, 0, hspi);
//				buttonHeldVisual(b4Box.x, b4Box.y, 0, hspi);
//				break;
//		}
		if (buttons.is1Pressed || buttons.is2Pressed || buttons.is3Pressed || buttons.is4Pressed) {
			if (buttons.is1Pressed) buttonTappedVisual(b1Box.x, b1Box.y, b1TapS=1-b1TapS, hspi);
			if (buttons.is2Pressed) buttonTappedVisual(b2Box.x, b2Box.y, b2TapS=1-b2TapS, hspi);
			if (buttons.is3Pressed) buttonTappedVisual(b3Box.x, b3Box.y, b3TapS=1-b3TapS, hspi);
			if (buttons.is4Pressed) buttonTappedVisual(b4Box.x, b4Box.y, b4TapS=1-b4TapS, hspi);
//			HAL_Delay(500);
			buttons.is1Pressed = buttons.is2Pressed = buttons.is3Pressed = buttons.is4Pressed = 0;
		}
//		else {
//			buttonTappedVisual(b1Box.x, b1Box.y, 0, hspi);
//			buttonTappedVisual(b2Box.x, b2Box.y, 0, hspi);
//			buttonTappedVisual(b3Box.x, b3Box.y, 0, hspi);
//			buttonTappedVisual(b4Box.x, b4Box.y, 0, hspi);
//		}

		char str[40];
		sprintf(str, "%d, %d, %d, %d", b1TapS, b2TapS, b3TapS, b4TapS);
		drawCenteredText(getDisplayWidth()/2, getDisplayHeight()-15, str, hspi);
	}
}

void layout(SPI_HandleTypeDef *hspi) {
	setDisplayOrientation(3, hspi);
	clearScreen(bgColor, hspi);

	drawTitle("Input Test", hspi);
	drawHLine(0, b1Box.y-5, getDisplayWidth(), ST77XX_BLACK, hspi);
	drawHLine(0, b4Box.y+vSpacing+5, getDisplayWidth(), ST77XX_BLACK, hspi);
	drawRect(b1Box.x, b1Box.y-1, hSpacing, vSpacing, ST77XX_BLACK, hspi);
	drawRect(b2Box.x, b2Box.y-1, hSpacing, vSpacing, ST77XX_BLACK, hspi);
	drawRect(b3Box.x, b3Box.y-1, hSpacing, vSpacing, ST77XX_BLACK, hspi);
	drawRect(b4Box.x, b4Box.y-1, hSpacing, vSpacing, ST77XX_BLACK, hspi);
	setTextSize(1);
	drawCenteredText(leftToCentered(b1Box.x, hSpacing), b1Box.y, "Button 1", hspi);
	drawCenteredText(leftToCentered(b2Box.x, hSpacing), b2Box.y, "Button 2", hspi);
	drawCenteredText(leftToCentered(b3Box.x, hSpacing), b3Box.y, "Button 3", hspi);
	drawCenteredText(leftToCentered(b4Box.x, hSpacing), b4Box.y, "Button 4", hspi);
}

void buttonTappedVisual(uint16_t boxX, uint16_t boxY, uint8_t isVisible, SPI_HandleTypeDef *hspi) {
	uint16_t x = boxX+hSpacing/2-15;
	uint16_t y = boxY+fontH+10;

	if (isVisible) fillRect(x, y, 10, 10, 0xDCC9, hspi);		// pale orange
	else fillRect(x, y, 10, 10, bgColor, hspi);
}

void buttonHeldVisual(uint16_t boxX, uint16_t boxY, uint8_t isVisible, SPI_HandleTypeDef *hspi) {
	uint16_t x = boxX+hSpacing/2-15;
	uint16_t y = boxY+fontH+10;

	if (isVisible) fillRect(x, y, 10, 10, 0x79BC, hspi);		// some purple
	else fillRect(x, y, 10, 10, bgColor, hspi);
}
