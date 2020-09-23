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
 */

#include "TFT_display.h"

static void SPI_TxISR_8BIT_circular(struct __SPI_HandleTypeDef *hspi);

// static variables
static uint8_t cursorX;			// cursor position for text drawing
static uint8_t cursorY;
static uint8_t textSize;		// size of characters
static uint16_t textColor;		// color of characters
static uint16_t bg;				// background color

// ---- lower level functions ----
void SPI_CS_LOW() {HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);}
void SPI_CS_HIGH() {HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);}
void SPI_DC_LOW() {HAL_GPIO_WritePin(DC_PORT, DC_PIN, GPIO_PIN_RESET);}
void SPI_DC_HIGH() {HAL_GPIO_WritePin(DC_PORT, DC_PIN, GPIO_PIN_SET);}

void sendCommand(uint8_t cmd, uint8_t *args, uint16_t numArgs, SPI_HandleTypeDef *hspi) {
	while (HAL_SPI_GetState(hspi) == HAL_SPI_STATE_BUSY_TX);		// block next transfer request while DMA transfer is ongoing
//	SPI_CS_LOW();	// chip select

	SPI_DC_LOW();	// command mode
	HAL_SPI_Transmit(hspi, &cmd, 1, 1000);	// not using DMA bc it's only 1 byte

	SPI_DC_HIGH();	// data mode
	if (numArgs) {
		HAL_SPI_Transmit_IT(hspi, args, numArgs);
	}
}

void sendColor(uint8_t cmd, uint16_t color, uint16_t numPixels, SPI_HandleTypeDef *hspi) {
	SPI_DC_LOW();
	HAL_SPI_Transmit_IT(hspi, &cmd, 1);
	SPI_DC_HIGH();
	uint16_t tempColor = color;		// scoping problem maybe
	SPI_Transmit_IT_1color(hspi, &tempColor, numPixels);
}

void SPI_Transmit_IT_1color(SPI_HandleTypeDef *hspi, uint16_t *pData, uint16_t size) {
	__HAL_LOCK(hspi);
	hspi->State = HAL_SPI_STATE_BUSY_TX;
	hspi->pTxBuffPtr = (uint8_t *)pData;
	hspi->TxXferSize = size;
	hspi->TxXferCount = size;

	hspi->pRxBuffPtr  = (uint8_t *)NULL;
	hspi->RxXferSize  = 0U;
	hspi->RxXferCount = 0U;
	hspi->RxISR       = NULL;

	hspi->TxISR = SPI_TxISR_8BIT_circular;
	__HAL_SPI_ENABLE_IT(hspi, (SPI_IT_TXE | SPI_IT_ERR));
	if ((hspi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE) {
		__HAL_SPI_ENABLE(hspi);
	}
	__HAL_UNLOCK(hspi);
}

static void SPI_TxISR_8BIT_circular(struct __SPI_HandleTypeDef *hspi) {
	*(__IO uint8_t *)&hspi->Instance->DR = (*hspi->pTxBuffPtr);
	hspi->TxXferCount % 2 == 0 ? hspi->pTxBuffPtr++ : hspi->pTxBuffPtr--;
	hspi->TxXferCount--;

	if (hspi->TxXferCount == 0) {
//		SPI_CloseTx_ISR(hspi);
		while ((hspi->Instance->SR & SPI_FLAG_TXE) == RESET);
		__HAL_SPI_DISABLE_IT(hspi, (SPI_IT_TXE | SPI_IT_ERR));
		hspi->State = HAL_SPI_STATE_READY;
#if (USE_HAL_SPI_REGISTER_CALLBACKS == 1U)
		hspi->TxCpltCallback(hspi);
#else
		HAL_SPI_TxCpltCallback(hspi);
#endif /* USE_HAL_SPI_REGISTER_CALLBACKS */
	}
}

// using only for sending data, but not commands
// dont send request when transfer is ongoing
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
//	if (HAL_GPIO_ReadPin(CS_PORT, CS_PIN) == GPIO_PIN_RESET) SPI_CS_HIGH();	// chip select disable
}

// array parser heavily based on Adafruit library code
void displayInit(uint8_t *args, SPI_HandleTypeDef *hspi) {
	uint8_t  numCommands, cmd, numArgs;
	uint16_t ms;
	uint8_t index = 0;
	uint8_t data;

	numCommands = args[index++];			// Number of commands to follow
	while(numCommands--) {					// For each command...
		cmd = args[index++];				// Read command
		numArgs  = args[index++];			// Number of args to follow
		ms       = numArgs & ST_CMD_DELAY;	// If hibit set, delay follows args
		numArgs &= ~ST_CMD_DELAY;			// Mask out delay bit
		sendCommand(cmd, &args[index], numArgs, hspi);
		index += numArgs;

		if(ms) {
			ms = args[index++];			// Read post-command delay time (ms)
			if(ms == 255) ms = 500;		// If 255, delay for 500 ms
			HAL_Delay(ms);
		}
	}

	data = 0xC0;
	sendCommand(ST77XX_MADCTL, &data, 1, hspi);
}

void TFT_startup(SPI_HandleTypeDef *hspi) {
	// array pulled from Adafruit's library for ST7735R driver
	uint8_t initCommands[] = {
		21,                             // 15 commands in list:
		ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
			150,                        //     150 ms delay
		ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
			255,                        //     500 ms delay
		ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
			0x01, 0x2C, 0x2D,           //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
			0x01, 0x2C, 0x2D,           //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
			0x01, 0x2C, 0x2D,           //     Dot inversion mode
			0x01, 0x2C, 0x2D,           //     Line inversion mode
		ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
			0x07,                       //     No inversion
		ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
			0xA2,
			0x02,                       //     -4.6V
			0x84,                       //     AUTO mode
		ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
			0xC5,                       //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
		ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
			0x0A,                       //     Opamp current small
			0x00,                       //     Boost frequency
		ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
			0x8A,                       //     BCLK/2,
			0x2A,                       //     opamp current small & medium low
		ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
			0x8A, 0xEE,
		ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
			0x0E,
		ST77XX_INVOFF,  0,              // 13: Don't invert display, no args
		ST77XX_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
			0xC8,                       //     row/col addr, bottom-top refresh
		ST77XX_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
			0x05,						//     16-bit color
		ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
			0x00, 0x00,                 //     XSTART = 0
			0x00, 0x7F,                 //     XEND = 127
		ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
			0x00, 0x00,                 //     XSTART = 0
			0x00, 0x9F,                 //     XEND = 159
		ST7735_GMCTRP1, 16      ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
			0x02, 0x1c, 0x07, 0x12,     //     (Not entirely necessary, but provides
			0x37, 0x32, 0x29, 0x2d,     //      accurate colors)
			0x29, 0x25, 0x2B, 0x39,
			0x00, 0x01, 0x03, 0x10,
		ST7735_GMCTRN1, 16      ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
			0x03, 0x1d, 0x07, 0x06,     //     (Not entirely necessary, but provides
			0x2E, 0x2C, 0x29, 0x2D,     //      accurate colors)
			0x2E, 0x2E, 0x37, 0x3F,
			0x00, 0x00, 0x02, 0x10,
		ST77XX_NORON,     ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
			10,                         //     10 ms delay
		ST77XX_DISPON,    ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
			100	                        //     100 ms delay
	};

	displayInit(initCommands, hspi);
	setAddrWindow(0, 0, WIDTH, HEIGHT, hspi);

	// set the global variables
	cursorX = 0;
	cursorY = 0;
	textSize = 1;
	textColor = ST77XX_BLACK;
	bg = ST77XX_WHITE;
}

// draw something: set addr window -> write to ram memory
// sets specific area on display to write pixels to
// x and y for upper left corner, w for width, h for height
void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SPI_HandleTypeDef *hspi) {
	// building 32-bit window args
	uint8_t temp[4];
	temp[0] = (x & (0xFF00)) >> 8;
	temp[1] = x & (0xFF);
	temp[2] = ((x+w-1) & (0xFF00)) >> 8;
	temp[3] = (x+w-1) & (0xFF);
	sendCommand(ST77XX_CASET, temp, 4, hspi);

	temp[0] = (y & (0xFF00)) >> 8;
	temp[1] = y & (0xFF);
	temp[2] = ((y+h-1) & (0xFF00)) >> 8;
	temp[3] = (y+h-1) & (0x00FF);
	sendCommand(ST77XX_RASET, temp, 4, hspi);
}

// sends turn on/off command
void turnDisplayOn(SPI_HandleTypeDef *hspi) {sendCommand(ST77XX_DISPON, NULL, 0, hspi);}
void turnDisplayOff(SPI_HandleTypeDef *hspi) {sendCommand(ST77XX_DISPOFF, NULL, 0, hspi);}
// ---- end of lower level functions

// ---- base graphics functions ----
// 8-bit spi bus wants msb first; in array, lowest index is sent first
// because ARM is little-endian
//   for 16-bit value, it sends lower byte before upper byte
//   resulting in device thinking lower byte is upper byte
// this switches byte order around
uint16_t colorFixer(uint16_t color) {
	uint8_t a = color & 0xFF;
	uint8_t b = (color & 0xFF00) >> 8;
	uint16_t ret = (a << 8) | b;

	return ret;
}

// draw a pixel on specified coordinates
void drawPixel(uint8_t x, uint8_t y, uint16_t color, SPI_HandleTypeDef *hspi) {
	// bounds checking
	// just don't draw if pixel is out of bounds
	if ((x > WIDTH) || (x < 0) || (y > HEIGHT) || (y < 0)) return;

	setAddrWindow(x, y, 1, 1, hspi);
	uint16_t tempColor = colorFixer(color);		// else we're using address of something passed by value
	sendCommand(ST77XX_RAMWR, &tempColor, 2, hspi);
}

// draw a horizontal line. coordinates are for left point
void drawHLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef *hspi) {
	// bounds checking
	if (x < 0) x = 0;						// don't set x out of bounds
	if (x > WIDTH) x = WIDTH;
	if (x+size > WIDTH) size = WIDTH-x;		// don't set size so line draws out of bounds
	if (x+size < 0) size = 0-x;
	if ((y > HEIGHT) || (y < 0)) return;	// don't draw if y is out of bounds

	setAddrWindow(x, y, size, 1, hspi);
	uint16_t colors[size];
	for (int i = 0; i < size; i++) {
		colors[i] = colorFixer(color);
	}

	sendCommand(ST77XX_RAMWR, colors, size*2, hspi);
}

// draws a vertical line. coordinates are for top point
void drawVLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef *hspi) {
	// bounds checking
	if (y < 0) y = 0;						// don't set y out of bounds
	if (y > HEIGHT) y = HEIGHT;
	if (y+size > HEIGHT) size = HEIGHT-y;	// don't set size so line draws out of bounds
	if (y+size < 0) size = 0-y;
	if ((x > WIDTH) || (x < 0)) return;		// don't draw if x is out of bounds

	setAddrWindow(x, y, 1, size, hspi);
	uint16_t colors[size];
	for (int i = 0; i < size; i++) {
		colors[i] = colorFixer(color);
	}

	sendCommand(ST77XX_RAMWR, colors, size*2, hspi);
}

// draws on a specific region with input 16-bit buffer
void drawBuffer(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t *buffer, uint16_t bufferSize, SPI_HandleTypeDef *hspi) {
	if (x+w > WIDTH || y+h > HEIGHT) return;

	// also don't call this with buffer size too big bc there's not enough ram for all pixels of display
	if (bufferSize > 10240) return;			// about 1/2 of total system ram

	setAddrWindow(x, y, w, h, hspi);
	sendCommand(ST77XX_RAMWR, buffer, bufferSize*2, hspi);
}
// ---- end of base graphics functions

// ---- basic shapes and lines ----
void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color, SPI_HandleTypeDef *hspi) {
	// based on Bresenham's line drawing algorithm (thx Adafruit and Wikipedia)
	if (x0 == x1) {
		drawVLine(x0, y0, abs(y0-y1), color, hspi);
		return;
	}
	if (y0 == y1) {
		drawHLine(x0, y0, abs(x0-y1), color, hspi);
		return;
	}

	int isSteep = abs(y1-y0) > abs(x1-x0);
	if (isSteep) {
		// swap x0-y0 and x1-y1 (swap x and y for slope reasons)
		swap_ints(x0, y0);
		swap_ints(x1, y1);
	}

	if (x0 > x1) {
		// swap x0-x1 and y0-y1 (reverse how we traverse line)
		swap_ints(x0, x1);
		swap_ints(y0, y1);
	}

	int ystep = (y0 < y1) ? 1 : -1;

	int dx = x1-x0;
	int dy = y1-y0;
	float slope = abs((float)dy/dx);

	float err = 0;
	int y = y0;
	for (int x = x0; x < x1; x++) {
		isSteep ? drawPixel(y, x, color, hspi) : drawPixel(x, y, color, hspi);
		err += slope;
		if (err >= 0.5) {
			y += ystep;
			err -= 1;
		}
	}
}

// draw an empty rectangle
void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef *hspi) {
	drawHLine(x, y, w, color, hspi);
	drawHLine(x, y+h-1, w, color, hspi);
	drawVLine(x, y, h, color, hspi);
	drawVLine(x+w-1, y, h, color, hspi);
}

// draw a filled rectangle
void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef *hspi) {
	for (int i = 0; i < h; i++) {
		drawHLine(x, y+i, w, color, hspi);
	}
}

// a big rectangle, but for the whole screen
void fillScreen(uint16_t color, SPI_HandleTypeDef *hspi) {
	setAddrWindow(0, 0, WIDTH, HEIGHT, hspi);
	color = colorFixer(color);
	sendColor(ST77XX_RAMWR, color, WIDTH*HEIGHT*2, hspi);
//	uint16_t bufferSize = WIDTH*HEIGHT/4;
//	uint16_t buffer[bufferSize];
//	int i;
//	for (i = 0; i < bufferSize; i++) {
//		buffer[i] = colorFixer(color);
//	}
//
//	// divided into 4 parts, since system ram is not big enough
//	for (i = 0; i < 4; i++) {
//		drawBuffer(0, HEIGHT/4*i, WIDTH, HEIGHT/4, buffer, bufferSize, hspi);
//	}
}

void clearScreen(uint16_t backgroundColor, SPI_HandleTypeDef *hspi) {
	bg = backgroundColor;
	fillScreen(backgroundColor, hspi);
}
// ---- end of basic shapes and lines ----

// ---- text functions ----
// draw a character. based on 6x8 font, but scalable
// instead of drawing pixel by pixel, function builds a buffer first and then sends
void drawChar(uint8_t ch, SPI_HandleTypeDef *hspi) {
	uint16_t bufferSize = 6*8*textSize*textSize;
	uint16_t buffer[bufferSize];
	int16_t rowOffset, address;

	// Char bitmap = 5 columns
	for (int8_t i=0; i<5; i++) {
		uint8_t line = font[ch*5+i];
		for (int8_t j=0; j<8; j++, line >>= 1) {
			// draw character pixel
			if (line & 1) {
				if (textSize == 1) {
					buffer[i+j*6] = colorFixer(textColor);
				}
				else {
					// indexing scheme for textSize > 1
					for (int8_t k = 0; k < textSize; k++) {
						rowOffset = textSize*6;
						for (int8_t l = 0; l < textSize; l++) {
							address = (textSize*textSize*j*6)+(i*textSize);
							address += rowOffset*k+l;
							buffer[address] = colorFixer(textColor);
						}
					}
				}
			}
			// draw text background
			else if (bg != textColor) {
				if (textSize == 1) {
					buffer[i+j*6] = colorFixer(bg);
				}
				else {
					for (int8_t k = 0; k < textSize; k++) {
						rowOffset = textSize*6;
						for (int8_t l = 0; l < textSize; l++) {
							address = (textSize*textSize*j*6)+(i*textSize);
							address += rowOffset*k+l;
							buffer[address] = colorFixer(bg);
						}
					}
				}
			}
		}
	}

	// If opaque, draw vertical line for last column
	// for character 1px kerning
	if (bg != textColor) {
		for (int8_t j = 0; j < 8; j++) {
			if (textSize == 1) {
				buffer[5+j*6] = colorFixer(bg);
			}
			else {
				for (int8_t k = 0; k < textSize; k++) {
					for (int8_t l = 0; l < textSize; l++) {
						address = (textSize*textSize*j*6)+(5*textSize);
						address += rowOffset*k+l;
						buffer[address] = colorFixer(bg);
					}
				}
			}
		}
	}

	drawBuffer(cursorX, cursorY, 6*textSize, 8*textSize, buffer, bufferSize, hspi);
}

// draws character strings
void drawText(const char *str, SPI_HandleTypeDef *hspi) {
	for (int i = 0; str[i] != '\0'; i++) {
		drawChar(str[i], hspi);

		// moves cursor on every character print so it's not printed to the same place
		setCursor(cursorX+textSize*6, cursorY);
	}
}

// drawText, but coordinates as arguments. coordinates are for upper left bound
void drawTextAt(uint8_t x, uint8_t y, const char *str, SPI_HandleTypeDef *hspi) {
	// add text wrap
	int i = 0;
	setCursor(x,y);
	for (i = 0; str[i] != '\0'; i++) {
		drawChar(str[i], hspi);
		setCursor(cursorX+textSize*6, cursorY);
	}
}

// draws text centered on an x coordinate. y is upper bound of box
void drawCenteredText(uint8_t x_center, uint8_t y, const char *str, SPI_HandleTypeDef *hspi) {
	uint8_t strSize = strlen(str);
	// bounds checking. text box needed to print text should not end up out of bounds
	// also calculating what bounds of text box should be
	if (y+textSize*8 > HEIGHT) return;
	int leftBound = x_center-(strSize*textSize*6)/2;
	int rightBound = x_center+(strSize*textSize*6)/2;
	if (leftBound < 0) return;
	if (rightBound > WIDTH) return;

	setCursor(leftBound, y);
	drawText(str, hspi);
}

// drawCenteredText, but background is filled to left and right of box
// used for cases where you're printing strings to the same place, but they have different sizes
void drawCenteredTextWithPadding(uint8_t x_center, uint8_t y, uint8_t maxLength, const char *str, SPI_HandleTypeDef *hspi) {
	// bounds checking. text box needed to print text should not end up out of bounds
	if (y+textSize*8 > HEIGHT) return;
	int leftBound = x_center-(maxLength*textSize*6)/2;
	int rightBound = x_center+(maxLength*textSize*6)/2;
	if (leftBound < 0) return;
	if (rightBound > WIDTH) return;

	uint8_t strSize = strlen(str);
	if (maxLength < strSize) return;		// size should not be greater than max

	// draw left and right padding
	uint8_t diff = maxLength-strSize;
	fillRect(leftBound, y, diff*textSize*6/2, textSize*8, bg, hspi);		// math out of order to accomodate diff/2 being a non-int
	fillRect(rightBound-diff*textSize*6/2, y, diff*textSize*6/2, textSize*8, bg, hspi);

	int textLeftBound = x_center-(strSize*textSize*6)/2;
	setCursor(textLeftBound, y);
	drawText(str, hspi);
}

// clear a line of text. y gives upper bound of text box
void clearTextLine(uint8_t y, SPI_HandleTypeDef *hspi) {
	fillRect(0, y, WIDTH, textSize*8, bg, hspi);
}

// ---- getters and setters ----
// sets static variables
void setBackgroundColor(uint16_t color) {bg = color;}

void setCursor(uint8_t x, uint8_t y) {
	cursorX = x;
	cursorY = y;
}

void setTextSize(uint8_t size) {textSize = size;}

void setTextColor(uint16_t color) {textColor = color;}

uint16_t getBackgroundColor() {return bg;}
uint16_t getTextColor() {return textColor;}
// ---- end of getters and setters ----
// ---- end of text functions ----
