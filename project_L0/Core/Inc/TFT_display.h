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
#include "font.h"			// include for byte array used with character drawing
#include <string.h>			// for strlen()

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
#define CS_PORT 	GPIOC
#define CS_PIN		GPIO_PIN_7
#define DC_PORT		GPIOB
#define DC_PIN		GPIO_PIN_6
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
void SPI_CS_LOW();
void SPI_CS_HIGH();
void SPI_DC_LOW();
void SPI_DC_HIGH();
void sendCommand(uint8_t cmd, uint8_t* args, uint16_t numArgs, SPI_HandleTypeDef *hspi);
void displayInit(uint8_t *args, SPI_HandleTypeDef *hspi);
void TFT_startup(SPI_HandleTypeDef *hspi);
void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SPI_HandleTypeDef *hspi);
void turnDisplayOn(SPI_HandleTypeDef *hspi);
void turnDisplayOff(SPI_HandleTypeDef *hspi);
// ---- End of lower level functions ----

// ---- Graphics functions ----
// ---- base functions ----
// convention: spi handle is last argument of funct
uint16_t colorFixer(uint16_t);
void drawPixel(uint8_t x, uint8_t y, uint16_t color, SPI_HandleTypeDef* hspi);
void drawHLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef* hspi);
void drawVLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef* hspi);
void drawBuffer(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t *buffer, uint16_t bufferSize, SPI_HandleTypeDef *hspi);
// ---- end of base functions ----

// ---- basic shapes and lines ----
// these functions are based completely on above functions
void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color, SPI_HandleTypeDef* hspi);
void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef* hspi);
void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef* hspi);
void fillScreen(uint16_t color, SPI_HandleTypeDef* hspi);
// ---- end of basic shapes and lines ----

// ---- other more complicated graphics ----
// pls sort this garbage. move some funcs to nav?
void drawCenteredText(uint8_t x_center, uint8_t y, const char *str, SPI_HandleTypeDef *hspi);
void drawCenteredTextWithPadding(uint8_t x_center, uint8_t y, uint8_t maxLength, const char *str, SPI_HandleTypeDef *hspi);
void clearTextLine(uint8_t y, SPI_HandleTypeDef *hspi);
// ---- end of other stuff ----

// ---- text functions ----
void drawChar(uint8_t ch, SPI_HandleTypeDef *hspi);
void drawText(const char *str, SPI_HandleTypeDef *hspi);
void drawTextAt(uint8_t x, uint8_t y, const char *str, SPI_HandleTypeDef *hspi);
void setBackgroundColor(uint16_t color);
void setCursor(uint8_t x, uint8_t y);
void setTextSize(uint8_t size);
void setTextColor(uint16_t color);
void clearScreen(uint16_t backgroundColor, SPI_HandleTypeDef* hspi);

uint16_t getBackgroundColor();
uint16_t getTextColor();
// ---- end of text functions ----

/* some more ideas for graphics:
 *   canvas system: a data structure or array representing the layout of the screen
 *     not sure if the L0 can hold the whole screen with 16-bit pixels because the L4 didn't show promising results
 *     declare as static so it can go into flash? might make things slower but might not really
 *       if we can do this, we can use DMA to do the whole screen transfer, and we'd need to do no work on it
 *     can use palette system so each pixel can be represented with much less bits (2, 4, or 8)
 *     can switch palettes on the fly (probably implied, why am i typing this out)
 *     rewrite graphics functions to work with this system
 *   draw bitmaps: import a file and read, or whatever
 *   draw other shapes: circles, triangles, polygons, rounded rectangles, etc.
 *     some are already in Adafruit library and would be easy to write in
 *     is it really necessary
 *   draw gradients: you know, maybe it'd be pretty
 *   display rotation: sometimes we might want display rotated to be horizontal instead of vertical
 *   invert display: quick and easy way to alter the whole screen for some sort of spooky effect
 *
 *   convenience:
 *     variables for cursor, textcolor, bg, textsize
 *     clear line
 *     newline/textwrap
 */

/* comments:
 *   character and text functions are slow and you can easily see characters scroll by
 *   due to a lot of checks and a lot of calls for each pixel in the character. character
 *   info needs very little memory, but how can we make this faster?
 *     retranslate 1-bit data structure into 16-bit data structure and pass as one call to SPI?
 *   is very slow on L0 nucleo
 *   ram is only 20kB, and can't use flash for variables that aren't const. 2-bit palette system and
 *     translate 1/4 of display at a time might fit into ram
 */
// ---- End of graphics functions ----
#endif
