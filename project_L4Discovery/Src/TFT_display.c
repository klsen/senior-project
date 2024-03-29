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
	// bounds checking
	// just don't draw if pixel is out of bounds
	if ((x > WIDTH) || (x < 0) || (y > HEIGHT) || (y < 0)) return;

	setAddrWindow(x, y, 1, 1, hspi);
	uint16_t tempColor = colorFixer(color);		// else we're using address of something passed by value
	sendCommand(ST77XX_RAMWR, &tempColor, 2, hspi);
}

void drawHLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef *hspi) {
	// bounds checking
	if (x < 0) x = 0;						// don't set x out of bounds
	if (x > WIDTH) x = WIDTH;
	if (x+size > WIDTH) size = WIDTH-x;		// don't set size so line draws out of bounds
	if (x+size < 0) size = 0-x;
	if ((y > HEIGHT) || (y < 0)) return;	// don't draw if y is out of bounds

	setAddrWindow(x, y, size, 1, hspi);
	uint16_t colors[size];
	for (int i = 0; i < size; i++) {		// better way to make array of 1 color; SPI without moving address of sent?
		colors[i] = colorFixer(color);
//		colors[i] = color;
	}

	sendCommand(ST77XX_RAMWR, colors, size*2, hspi);
}

void drawVLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef *hspi) {
	// bounds checking
	if (y < 0) x = 0;						// don't set x out of bounds
	if (y > HEIGHT) x = HEIGHT;
	if (y+size > HEIGHT) size = HEIGHT-y;	// don't set size so line draws out of bounds
	if (y+size < 0) size = 0-y;
	if ((x > WIDTH) || (x < 0)) return;		// don't draw if y is out of bounds

	setAddrWindow(x, y, 1, size, hspi);
	uint16_t colors[size];
	for (int i = 0; i < size; i++) {		// there has to be a better way to make array of 1 color
											// SPI without moving address of sent buffer?
		colors[i] = colorFixer(color);
	}

	sendCommand(ST77XX_RAMWR, colors, size*2, hspi);
}

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

void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef *hspi) {
	drawHLine(x, y, w, color, hspi);
	drawHLine(x, y+h-1, w, color, hspi);
	drawVLine(x, y, h, color, hspi);
	drawVLine(x+w-1, y, h, color, hspi);
}

void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef *hspi) {
	for (int i = 0; i < h; i++) {
		drawHLine(x, y+i, w, color, hspi);
	}
}

void fillScreen(uint16_t color, SPI_HandleTypeDef *hspi) {
	fillRect(0, 0, WIDTH, HEIGHT, color, hspi);
}

void drawChar(uint8_t x, uint8_t y, uint8_t ch, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y, SPI_HandleTypeDef *hspi) {
	// very much ripped from Adafruit
	// saves me a lot of time from making my own fonts
	// thinking of only using 1 parameter for size?

//	if((x >= _width)            || // Clip right
//	   (y >= _height)           || // Clip bottom
//	   ((x + 6 * size_x - 1) < 0) || // Clip left
//	   ((y + 8 * size_y - 1) < 0))   // Clip top
//		return;

//	if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

	for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
		uint8_t line = font[ch*5+i];
		for(int8_t j=0; j<8; j++, line >>= 1) {
			if(line & 1) {
				if(size_x == 1 && size_y == 1)
					drawPixel(x+i, y+j, color, hspi);
				else
					fillRect(x+i*size_x, y+j*size_y, size_x, size_y, color, hspi);
			} else if(bg != color) {
				if(size_x == 1 && size_y == 1)
					drawPixel(x+i, y+j, bg, hspi);
				else
					fillRect(x+i*size_x, y+j*size_y, size_x, size_y, bg, hspi);
			}
		}
	}
	if(bg != color) { // If opaque, draw vertical line for last column
		if(size_x == 1 && size_y == 1) drawVLine(x+5, y, 8, bg, hspi);
		else          fillRect(x+5*size_x, y, size_x, 8*size_y, bg, hspi);
	}
}

// this function is slow, and you can definitely see a scrolling speed thing going on
// how to remove this so it prints near instantly?
// maybe not needed if all we're doing is printing time (very few characters)
void drawText(uint8_t x, uint8_t y, uint8_t size, uint16_t color, char *str, SPI_HandleTypeDef *hspi) {
	int strsize = 0;
	for (int i = 0; str[i] != '\0'; i++) {
		strsize++;
	}
	fillRect(x, y, strsize*size*6, size*8, ST77XX_WHITE, hspi);
	// add text wrap
	for (int i = 0; i < strsize; i++) {
		drawChar(x+i*6*size, y, str[i], color, ST77XX_BLACK, size, size, hspi);
	}
}
