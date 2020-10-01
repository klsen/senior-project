/*
 * Basic graphics library for using the Adafruit TFT LCD display
 * with the ST7735R driver chip and SPI. Based heavily on Adafruit
 * Arduino library for the same display.
 *
 * Can:
 *   Initialize display
 *   Draw lines between 2 points
 *   Draw rectangles
 *   Print characters
 *   set text color, background color, and text size
 */

#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include "stm32l0xx_hal.h"
//#include "stm32l0xx_ll_spi.h"
#include "font.h"			// include for byte array used with character drawing
#include <string.h>			// for strlen()
#include <stdlib.h>			// for abs()

// ---- driver command constants ----
// pulled from Adafruit ST77XX driver library.
// read ST7735R data sheet for more info on these constants
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04

#define ST7735_FRMCTR1    0xB1
#define ST7735_FRMCTR2    0xB2
#define ST7735_FRMCTR3    0xB3
#define ST7735_INVCTR     0xB4
#define ST7735_DISSET5    0xB6

#define ST7735_PWCTR1     0xC0
#define ST7735_PWCTR2     0xC1
#define ST7735_PWCTR3     0xC2
#define ST7735_PWCTR4     0xC3
#define ST7735_PWCTR5     0xC4
#define ST7735_VMCTR1     0xC5

#define ST7735_PWCTR6     0xFC

#define ST7735_GMCTRP1    0xE0
#define ST7735_GMCTRN1    0xE1

#define ST7735_TFTWIDTH_128   128 // for 1.44 and mini
#define ST7735_TFTWIDTH_80     80 // for mini
#define ST7735_TFTHEIGHT_128  128 // for 1.44" display
#define ST7735_TFTHEIGHT_160  160 // for 1.8" and mini display

#define ST_CMD_DELAY      0x80    // special signifier for command lists

#define ST77XX_NOP        0x00
#define ST77XX_SWRESET    0x01
#define ST77XX_RDDID      0x04
#define ST77XX_RDDST      0x09

#define ST77XX_SLPIN      0x10
#define ST77XX_SLPOUT     0x11
#define ST77XX_PTLON      0x12
#define ST77XX_NORON      0x13

#define ST77XX_INVOFF     0x20
#define ST77XX_INVON      0x21
#define ST77XX_DISPOFF    0x28
#define ST77XX_DISPON     0x29
#define ST77XX_CASET      0x2A
#define ST77XX_RASET      0x2B
#define ST77XX_RAMWR      0x2C
#define ST77XX_RAMRD      0x2E

#define ST77XX_PTLAR      0x30
#define ST77XX_TEOFF      0x34
#define ST77XX_TEON       0x35
#define ST77XX_MADCTL     0x36
#define ST77XX_COLMOD     0x3A

#define ST77XX_MADCTL_MY  0x80
#define ST77XX_MADCTL_MX  0x40
#define ST77XX_MADCTL_MV  0x20
#define ST77XX_MADCTL_ML  0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1      0xDA
#define ST77XX_RDID2      0xDB
#define ST77XX_RDID3      0xDC
#define ST77XX_RDID4      0xDD
// ---- end of driver constants ----

// ---- 16-bit (5-6-5) color constants ----
#define	ST77XX_BLACK      0x0000
#define ST77XX_WHITE      0xFFFF
#define	ST77XX_RED        0xF800
#define	ST77XX_GREEN      0x07E0
#define	ST77XX_BLUE       0x001F
#define ST77XX_CYAN       0x07FF
#define ST77XX_MAGENTA    0xF81F
#define ST77XX_YELLOW     0xFFE0
#define	ST77XX_ORANGE     0xFC00
// ---- End of 16-bit color constants ----

// ---- pin definitions and constants ----
#define CS_PORT 	GPIOB
#define CS_PIN		GPIO_PIN_12
#define DC_PORT		GPIOB
#define DC_PIN		GPIO_PIN_14
#define _xstart		0
#define _ystart 	0

#define HEIGHT 		160
#define WIDTH 		128
// ---- End of pin definitions and constants ----

// ---- Function Macros ----
#define swap_ints(x,y) (x)=(x)+(y); (y)=(x)-(y); (x)=(x)-(y)
//#define abs(x) ((x) > 0 ? (x) : -(x))
// ---- End of function macros ----

// ---- Lower level functions ----
// toggles gpio pins for SPI chip select and data/command pin on ST77XX driver
void SPI_CS_LOW();
void SPI_CS_HIGH();
void SPI_DC_LOW();
void SPI_DC_HIGH();

// sendCommand, TFT_startup, displayInit, and setAddrWindow are heavily based code from Adafruit's Arduino library
// sends ST77XX instruction and its necessary arguments
void sendCommand(uint8_t cmd, uint8_t* args, uint16_t numArgs, SPI_HandleTypeDef *hspi);
void sendColor(uint16_t color, uint16_t numPixels, SPI_HandleTypeDef *hspi);

// runs initialization for display. contains an array with a list of arguments for initialization
void TFT_startup(SPI_HandleTypeDef *hspi);

// array parser for TFT_startup
void displayInit(uint8_t *args, SPI_HandleTypeDef *hspi);

// sets display region that incoming pixel data should write to
void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SPI_HandleTypeDef *hspi);

// turns display on/off by sending ST77XX instruction
void turnDisplayOn(SPI_HandleTypeDef *hspi);
void turnDisplayOff(SPI_HandleTypeDef *hspi);
// ---- End of lower level functions ----

// ---- Graphics functions ----
// ---- base functions ----
// convention: spi handle is last argument of funct
uint16_t colorFixer(uint16_t);		// helper function involved with 16-bit pixel data and 8-bit SPI data bus

// draws pixels, lines, and an input 16-bit buffer
// coordinates are for upper left corner. screen is oriented vertically with pins on bottom by default
// hline x and y are for left point, vline x and y are for top point
void drawPixel(uint8_t x, uint8_t y, uint16_t color, SPI_HandleTypeDef* hspi);
void drawHLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef* hspi);
void drawVLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef* hspi);
void drawBuffer(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t *buffer, uint16_t bufferSize, SPI_HandleTypeDef *hspi);
// ---- end of base functions ----

// ---- basic shapes and lines ----
// these functions are based completely on above functions
// draws lines and rectangles
void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef* hspi);
void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef* hspi);

// draws a line from point to point
void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color, SPI_HandleTypeDef* hspi);

// completely fills screen with 1 color
void fillScreen(uint16_t color, SPI_HandleTypeDef* hspi);

// like fillScreen, but sets background color
void clearScreen(uint16_t backgroundColor, SPI_HandleTypeDef* hspi);
// ---- end of basic shapes and lines ----

// ---- text functions ----
// functions use textColor, textSize, cursor, and backgroundColor functions
// draws any ASCII character. based on characters mapped in font.h. printout in font.txt
void drawChar(uint8_t ch, SPI_HandleTypeDef *hspi);

// draws a string
void drawText(const char *str, SPI_HandleTypeDef *hspi);

// drawText, but coordinates specified
void drawTextAt(uint8_t x, uint8_t y, const char *str, SPI_HandleTypeDef *hspi);

// text but centered on screen
void drawCenteredText(uint8_t x_center, uint8_t y, const char *str, SPI_HandleTypeDef *hspi);

// centered text, but left and right spaces are filled
// used for printing text in the same space with different sizes
void drawCenteredTextWithPadding(uint8_t x_center, uint8_t y, uint8_t maxLength, const char *str, SPI_HandleTypeDef *hspi);

// clears a line of text at a given upper bound point
void clearTextLine(uint8_t y, SPI_HandleTypeDef *hspi);
// ---- end of text functions ----

// ---- setters and getters ----
// used in text functions to color behind characters
void setBackgroundColor(uint16_t color);

// sets cursor position. used in drawText
void setCursor(uint8_t x, uint8_t y);

// sets text size to scale characters. input is a multiplier on 6x8.
void setTextSize(uint8_t size);

// sets color of text
void setTextColor(uint16_t color);

// returns static variables that hold background color and text color
uint16_t getBackgroundColor();
uint16_t getTextColor();
// ---- end of setters and getters ----
// ---- End of graphics functions ----
#endif
