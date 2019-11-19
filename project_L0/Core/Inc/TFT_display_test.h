/*
 * Tests graphics functions for the Adafruit TFTLCD display,
 * which uses the ST7735R driver chip and communicates using SPI.
 */

#ifndef TFT_DISPLAY_TEST_H
#define TFT_DISPLAY_TEST_H

#include "stm32l0xx_hal.h"
#include "TFT_display.h"

// draws diagonal, horizontal, and vertical lines
void lineTest(SPI_HandleTypeDef *hspi);

// draws all characters in the character set in different colors
// and different sizes
void charTest(SPI_HandleTypeDef *hspi);

// draws small bits of text using a char*
void textTest(SPI_HandleTypeDef *hspi);

// draws pixels, horizontal lines, and vertical lines
void basicTest(SPI_HandleTypeDef *hspi);

#endif
