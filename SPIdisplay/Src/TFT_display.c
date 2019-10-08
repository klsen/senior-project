// code ripped from Adafruit library

#include "TFT_display.h"

void SPI_CS_LOW() {HAL_GPIO_WritePin(CS_GPIO, CS_PIN, GPIO_PIN_RESET);}

void SPI_CS_HIGH() {HAL_GPIO_WritePin(CS_GPIO, CS_PIN, GPIO_PIN_SET);}

void SPI_DC_LOW() {HAL_GPIO_WritePin(DC_GPIO, DC_PIN, GPIO_PIN_RESET);}

void SPI_DC_HIGH() {HAL_GPIO_WritePin(DC_GPIO, DC_PIN, GPIO_PIN_SET);}

// still ripped but now im changing it
void sendCommand(uint8_t cmd, uint8_t *args, uint16_t numArgs, SPI_HandleTypeDef *hspi) {
	SPI_CS_LOW();	// chip select

	SPI_DC_LOW();	// command mode
	HAL_SPI_Transmit(hspi, &cmd, 1, 1000);

	SPI_DC_HIGH();	// data mode
	if (numArgs) {
		HAL_SPI_Transmit(hspi, args, numArgs, 1000);
	}

	SPI_CS_HIGH();	// chip select disable
}

// array parser heavily based on Adafruit library code
void displayInit(uint8_t *args, SPI_HandleTypeDef *hspi) {
	uint8_t  numCommands, cmd, numArgs;
	uint16_t ms;
	uint8_t index = 0;
	uint8_t data;

	numCommands = args[index++];   // Number of commands to follow
	while(numCommands--) {                 // For each command...
		cmd = args[index++];         // Read command
		numArgs  = args[index++];    // Number of args to follow
		ms       = numArgs & ST_CMD_DELAY;   // If hibit set, delay follows args
		numArgs &= ~ST_CMD_DELAY;            // Mask out delay bit
		sendCommand(cmd, &args[index], numArgs, hspi);
		index += numArgs;

		if(ms) {
			ms = args[index++]; // Read post-command delay time (ms)
			if(ms == 255) ms = 500;     // If 255, delay for 500 ms
			HAL_Delay(ms);
		}
	}

	// note: this line may be doing more work than I think
	// (hard to read in datasheet)
	data = 0xC0;
	sendCommand(ST77XX_MADCTL, &data, 1, hspi);
}

void TFT_startup(SPI_HandleTypeDef *hspi) {
	// startup sequence: rcmd1->rcmd2red->rcmd3
	uint8_t initCommands[] = {
		21,                             // 15 commands in list:
		ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
			150,                          //     150 ms delay
		ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
			255,                          //     500 ms delay
		ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
			0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
			0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
			0x01, 0x2C, 0x2D,             //     Dot inversion mode
			0x01, 0x2C, 0x2D,             //     Line inversion mode
		ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
			0x07,                         //     No inversion
		ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
			0xA2,
			0x02,                         //     -4.6V
			0x84,                         //     AUTO mode
		ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
			0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
		ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
			0x0A,                         //     Opamp current small
			0x00,                         //     Boost frequency
		ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
			0x8A,                         //     BCLK/2,
			0x2A,                         //     opamp current small & medium low
		ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
			0x8A, 0xEE,
		ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
			0x0E,
		ST77XX_INVOFF,  0,              // 13: Don't invert display, no args
		ST77XX_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
			0xC8,                         //     row/col addr, bottom-top refresh
		ST77XX_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
			0x05,						//     16-bit color
		ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
			0x00, 0x00,                   //     XSTART = 0
			0x00, 0x7F,                   //     XEND = 127
		ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
			0x00, 0x00,                   //     XSTART = 0
			0x00, 0x9F,                 //     XEND = 159
		ST7735_GMCTRP1, 16      ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
			0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
			0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
			0x29, 0x25, 0x2B, 0x39,
			0x00, 0x01, 0x03, 0x10,
		ST7735_GMCTRN1, 16      ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
			0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
			0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
			0x2E, 0x2E, 0x37, 0x3F,
			0x00, 0x00, 0x02, 0x10,
		ST77XX_NORON,     ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
			10,                           //     10 ms delay
		ST77XX_DISPON,    ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
			100	                        //     100 ms delay
	};

	displayInit(initCommands, hspi);
	setAddrWindow(0, 0, WIDTH, HEIGHT, hspi);
}

// draw something: set addr window -> write to ram memory
// sets specific area on display to write pixels to
// x and y for upper left corner, w for width, h for height
void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SPI_HandleTypeDef *hspi) {
	// not really needed for our display
	x += _xstart;
	y += _ystart;

//	uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
//	uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
//	xa = toBigEndian_32(xa);
//	sendCommand(ST77XX_CASET, xa, 4, hspi);
//	ya = toBigEndian_32(ya);
//	sendCommand(ST77XX_RASET, ya, 4, hspi);

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

//uint32_t toBigEndian_32(uint32_t x) {
//	uint8_t temp;
//	// grab bottom 8 for temp
//	// set bottom 8 from top 8
//	// set top 8 with temp
//	for (int i = 0; i < 2; i++) {
//		temp = x & (0xFF << (i*8));
//
//		x &= ~(0xFF << (i*8));						// clear bottom 8
//		x |= (x & (0xFF << ((3-i)*8))) >> ((3-i)*8);	// set bottom 8 from top 8
//
//		x &= ~(0xFF << (3-i)*8);		// clear top 8
//		x |= temp << (3-i)*8;			// set top 8 with temp;
//	}
//	return x;
//}

// 8-bit spi bus wants msb first; in array, lowest index is sent first
// because L4 is little-endian
//   for 16-bit value, it sends lower byte before upper byte
//   resulting in device thinking lower byte is upper byte
// this switches byte order around
uint16_t colorFixer(uint16_t color) {
	uint8_t a = color & 0xFF;
	uint8_t b = (color & 0xFF00) >> 8;
	uint16_t ret = (a << 8) | b;

	return ret;
}

void drawPixel(uint8_t x, uint8_t y, uint16_t color, SPI_HandleTypeDef *hspi) {
	setAddrWindow(x, y, 1, 1, hspi);
	uint16_t tempColor = colorFixer(color);		// else we're using address of something passed by value
//	tempColor = color;
	sendCommand(ST77XX_RAMWR, &tempColor, 2, hspi);
}

void drawHLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef *hspi) {
	setAddrWindow(x, y, size, 1, hspi);
	uint16_t colors[size];
	for (int i = 0; i < size; i++) {		// better way to make array of 1 color; SPI without moving address of sent?
		colors[i] = colorFixer(color);
//		colors[i] = color;
	}

	sendCommand(ST77XX_RAMWR, colors, size*2, hspi);
}

void drawVLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef *hspi) {
	setAddrWindow(x, y, 1, size, hspi);
	uint16_t colors[size];
	for (int i = 0; i < size; i++) {		// there has to be a better way to make array of 1 color
											// SPI without moving address of sent buffer?
		colors[i] = colorFixer(color);
//		colors[i] = color;
	}

	sendCommand(ST77XX_RAMWR, colors, size*2, hspi);
}

void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef *hspi) {
	for (int i = 0; i < h; i++) {
		drawHLine(x, y+i, w, color, hspi);
	}

//	// oh my god this version is really slow
//	for (int i = 0; i < h; i++) {
//		for (int j = 0; j < w; j++) {
//			drawPixel(x+j, y+i, color, hspi);
//		}
//	}
}

//void swap_ints2(int *x, int *y) {
//	*x = *x + *y;
//	*y = *x - *y;
//	*x = *x - *y;
//}

//uint8_t abs(int x) {
//	return x > 0 ? x : -x;
//}

void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color, SPI_HandleTypeDef *hspi) {
	if (x0 == x1) {
		drawVLine(x0, y0, abs(y0-y1), color, hspi);
		return;
	}
	if (y0 == y1) {
		drawHLine(x0, y0, abs(x0-y1), color, hspi);
		return;
	}

	if (x0 > WIDTH) 	x0 = WIDTH;
	if (x0 < 0) 		x0 = 0;
	if (x1 > WIDTH) 	x1 = WIDTH;
	if (x1 < 0) 		x1 = 0;
	if (y0 > HEIGHT) 	y0 = HEIGHT;
	if (y0 < 0) 		y0 = 0;
	if (y1 > HEIGHT)	y1 = HEIGHT;
	if (y1 < 0) 		y1 = 0;

	// standard ints vs 8-bit ints? memory cost, convenience
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

void fillScreen(uint16_t color, SPI_HandleTypeDef *hspi) {
//	color = colorFixer(color);
//	for (int i = 0; i < HEIGHT; i++) {
//		drawHLine(0, i, WIDTH, color, hspi);
//	}
	drawRect(0, 0, WIDTH, HEIGHT, color, hspi);
}
