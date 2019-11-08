/*
 * Tests graphics functions for the Adafruit TFTLCD display,
 * which uses the ST7735R driver chip and communicates using SPI.
 */

#include "TFT_display_test.h"

void lineTest(uint16_t bg, SPI_HandleTypeDef *hspi) {
	int x = WIDTH/2;
	int y = HEIGHT/2;

	drawLine(x, y, x+WIDTH/2, y+HEIGHT/4, ST77XX_RED, hspi);
	HAL_Delay(500);
	drawRect(x, y, WIDTH/2, HEIGHT/4, ST77XX_RED, hspi);
	HAL_Delay(1000);

	drawLine(x, y, x+WIDTH/4, y+HEIGHT/2, ST77XX_BLUE, hspi);
	HAL_Delay(500);
	drawRect(x, y, WIDTH/4, HEIGHT/2, ST77XX_BLUE, hspi);
	HAL_Delay(1000);

	drawLine(x, y, x-WIDTH/4, y+HEIGHT/2, ST77XX_YELLOW, hspi);
	HAL_Delay(500);
	drawRect(x-WIDTH/4, y, WIDTH/4, HEIGHT/2, ST77XX_YELLOW, hspi);
	HAL_Delay(1000);

	drawLine(x, y, x-WIDTH/2, y+HEIGHT/4, ST77XX_GREEN, hspi);
	HAL_Delay(500);
	drawRect(x-WIDTH/2, y, WIDTH/2, HEIGHT/4, ST77XX_GREEN, hspi);
	HAL_Delay(1000);

	drawLine(x, y, x-WIDTH/2, y-HEIGHT/4, ST77XX_ORANGE, hspi);
	HAL_Delay(500);
	drawRect(x-WIDTH/2, y-HEIGHT/4, WIDTH/2, HEIGHT/4, ST77XX_ORANGE, hspi);
	HAL_Delay(1000);

	drawLine(x, y, x-WIDTH/4, y-HEIGHT/2, ST77XX_MAGENTA, hspi);
	HAL_Delay(500);
	drawRect(x-WIDTH/4, y-HEIGHT/2, WIDTH/4, HEIGHT/2, ST77XX_MAGENTA, hspi);
	HAL_Delay(1000);

	drawLine(x, y, x+WIDTH/4, y-HEIGHT/2, ST77XX_CYAN, hspi);
	HAL_Delay(500);
	drawRect(x, y-HEIGHT/2, WIDTH/4, HEIGHT/2, ST77XX_CYAN, hspi);
	HAL_Delay(1000);

	drawLine(x, y, x+WIDTH/2, y-HEIGHT/4, ST77XX_WHITE, hspi);
	HAL_Delay(500);
	drawRect(x, y-HEIGHT/4, WIDTH/2, HEIGHT/4, ST77XX_WHITE, hspi);
	HAL_Delay(1000);

	fillScreen(bg, hspi);
}

void charTest(uint16_t bg, SPI_HandleTypeDef *hspi) {
	uint8_t x, y;

	uint16_t rainbowColors[] = {
		ST77XX_RED,
		ST77XX_ORANGE,
		ST77XX_YELLOW,
		ST77XX_GREEN,
		ST77XX_CYAN,
		ST77XX_BLUE,
		ST77XX_MAGENTA
	};

	// print the standard 127 6x8 characters in different sizes
	for (int ch_size = 1; ch_size < 6; ch_size++) {
		for (unsigned char ch = 0; ch < 255; ch++) {
			// move to right enough for next char
			x = (ch*ch_size*6) % (WIDTH/(ch_size*6)*(ch_size*6));
			// line break when x gets near WIDTH
			y = ((8*ch_size) * ((ch*ch_size*6) / (WIDTH/(ch_size*6)*(ch_size*6)))) % (HEIGHT/(ch_size*8)*(ch_size*8));

			drawChar(x, y, ch, rainbowColors[ch%7], bg, ch_size, ch_size, hspi);
//			HAL_Delay(250);
		}
		HAL_Delay(1000);
		fillScreen(bg, hspi);
	}
}

void textTest(uint16_t bg, SPI_HandleTypeDef *hspi) {
	char *str = "Hello";
	drawText(0, 0, 1, ST77XX_BLACK, bg, str, hspi);
	HAL_Delay(1000);

	str = "Looooooooooooooooooooooooooooooong string";
	drawText(0, 16, 1, ST77XX_BLACK, bg, str, hspi);
	HAL_Delay(1000);

	str = "more text";
	drawText(0, 40, 1, ST77XX_BLACK, bg, str, hspi);
	HAL_Delay(1000);

	str = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	drawText(0, 64, 1, ST77XX_BLACK, bg, str, hspi);
	HAL_Delay(500);

	fillScreen(bg, hspi);
}

void basicTest(uint16_t bg, SPI_HandleTypeDef *hspi) {
	drawPixel(0, 0, ST77XX_BLUE, hspi);
	HAL_Delay(250);
	drawHLine(0, 159, 128, ST77XX_RED, hspi);
	HAL_Delay(250);
	drawVLine(127, 0, 160, ST77XX_GREEN, hspi);
	drawRect(50, 50, 50, 50, ST77XX_CYAN, hspi);

	HAL_Delay(500);

	fillScreen(bg, hspi);
	drawPixel(0, 0, bg, hspi);
	drawHLine(0, 159, 128, bg, hspi);
	drawVLine(127, 0, 160, bg, hspi);
	drawRect(50, 50, 50, 50, bg, hspi);
}
