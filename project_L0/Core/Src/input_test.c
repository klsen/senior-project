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
static uint16_t hSpacing = getDisplayWidth()/4;
static uint16_t vSpacing = getDisplayHeight()/4;
static struct coords b1Box = {getDisplayWidth()/4, getDisplayHeight()/4};
static struct coords b2Box = {b1Box.x+hSpacing, b1Box.y};
static struct coords b3Box = {b1Box.x, b1Box.y+vSpacing};
static struct coords b4Box = {b1Box.x+hSpacing, b1Box.y+vSpacing};

void input_test(SPI_HandleTypeDef *hspi) {
	static uint8_t b1TapS, b1HoldS, b2TapS, b2HoldS, b3TapS, b3HoldS, b4TapS, b4HoldS;

	layout(hspi);

	while (1) {
		// add code about buttons here
		// TODO: code about pulling detected inputs
		switch(input) {
			case b1Tap:
				buttonTappedVisual(b1Box.x, b1Box.y, 1, hspi);
				break;
			case b1Hold:
				buttonHeldVisual(b1Box.x, b1Box.y, 1, hspi);
				break;
			case b2Tap:
				buttonTappedVisual(b2Box.x, b2Box.y, 1, hspi);
				break;
			case b2Hold:
				buttonHeldVisual(b2Box.x, b2Box.y, 1, hspi);
				break;
			case b3Tap:
				buttonTappedVisual(b3Box.x, b3Box.y, 1, hspi);
				break;
			case b3Hold:
				buttonHeldVisual(b3Box.x, b3Box.y, 1, hspi);
				break;
			case b4Tap:
				buttonTappedVisual(b4Box.x, b4Box.y, 1, hspi);
				break;
			case b4Hold:
				buttonHeldVisual(b4Box.x, b4Box.y, 1, hspi);
				break;
			default:
				buttonTappedVisual(b1Box.x, b1Box.y, 0, hspi);
				buttonTappedVisual(b2Box.x, b2Box.y, 0, hspi);
				buttonTappedVisual(b3Box.x, b3Box.y, 0, hspi);
				buttonTappedVisual(b4Box.x, b4Box.y, 0, hspi);
				buttonHeldVisual(b1Box.x, b1Box.y, 0, hspi);
				buttonHeldVisual(b2Box.x, b2Box.y, 0, hspi);
				buttonHeldVisual(b3Box.x, b3Box.y, 0, hspi);
				buttonHeldVisual(b4Box.x, b4Box.y, 0, hspi);
				break;
		}
	}
}

void layout(SPI_HandleTypeDef *hspi) {
	setDisplayOrientation(3, hspi);
	clearScreen(bgColor, hspi);

	drawTitle("Input Test", hspi);
	drawHLine(0, b1Box.y-5, getDisplayWidth(), ST77XX_BLACK, hspi);
	drawHLine(0, b4Box.y+vSpacing+5, getDisplayWidth(), ST77XX_BLACK, hspi);
	drawCenteredText(leftToCentered(b1Box.x, hSpacing), b1Box.y, "Button 1", hspi);
	drawCenteredText(leftToCentered(b2Box.x, hSpacing), b2Box.y, "Button 2", hspi);
	drawCenteredText(leftToCentered(b3Box.x, hSpacing), b3Box.y, "Button 3", hspi);
	drawCenteredText(leftToCentered(b4Box.x, hSpacing), b4Box.y, "Button 4", hspi);
}

void buttonTappedVisual(uint16_t boxX, uint16_t boxY, uint8_t isVisible, SPI_HandleTypeDef *hspi) {
	uint16_t x = boxX-15;
	uint16_t y = boxY+fontH+10;

	if (isVisible) drawRect(x, y, 10, 10, 0xDCC9, hspi);		// pale orange
	else drawRect(x, y, 10, 10, bgColor, hspi);
}

void buttonHeldVisual(uint16_t boxX, uint16_t boxY, uint8_t isVisible, SPI_HandleTypeDef *hspi) {
	uint16_t x = boxX-15;
	uint16_t y = boxY+fontH+10;

	if (isVisible) drawRect(x, y, 10, 10, 0x79BC, hspi);		// some purple
	else drawRect(x, y, 10, 10, bgColor, hspi);
}
