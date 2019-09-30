// code ripped from Adafruit library
//#include "stm32l4xx_hal.h"
#include "TFT_display.h"
// startup sequence: rcmd1->rcmd2red->rcmd3
uint8_t Rcmd1[] = {                       // 7735R init, part 1 (red or green tab)
    15,                             // 15 commands in list:
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
      	0x05						//     16-bit color
};

uint8_t Rcmd2green[] = {                  // 7735R init, part 2 (green tab only)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
     	0x00, 0x02,                   //     XSTART = 0
     	0x00, 0x7F+0x02,              //     XEND = 127
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
     	0x00, 0x01,                   //     XSTART = 0
      	0x00, 0x9F+0x01 };            //     XEND = 159

uint8_t Rcmd2red[] = {                    // 7735R init, part 2 (red tab only)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      	0x00, 0x00,                   //     XSTART = 0
     	0x00, 0x7F,                   //     XEND = 127
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
     	0x00, 0x00,                   //     XSTART = 0
     	0x00, 0x9F };                 //     XEND = 159

uint8_t Rcmd3[] = {                       // 7735R init, part 3 (red or green tab)
    4,                              //  4 commands in list:
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
      	100 };                        //     100 ms delay

// draw something: set addr window -> write to ram memory
void setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
	uint16_t h, SPI_HandleTypeDef *hspi) {
	x += _xstart;
	y += _ystart;
	//uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
	//uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
	uint8_t temp[4];

	//writeCommand(ST77XX_CASET); // Column addr set
	//SPI_WRITE32(xa);
//	temp[0] = x;
//	temp[1] = x+w-1;
	temp[0] = (x & (0xFF00)) >> 8;
	temp[1] = x & (0xFF);
	temp[2] = ((x+w-1) & (0xFF00)) >> 8;
	temp[3] = (x+w-1) & (0xFF);
	sendCommand(ST77XX_CASET, temp, 4, hspi);

	//writeCommand(ST77XX_RASET); // Row addr set
	//SPI_WRITE32(ya);
//	temp[0] = y;
//	temp[1] = y+w-1;
	temp[0] = (y & (0xFF00)) >> 8;
	temp[1] = y & (0xFF);
	temp[2] = ((y+h-1) & (0xFF00)) >> 8;
	temp[3] = (y+h-1) & (0x00FF);
	sendCommand(ST77XX_RASET, temp, 4, hspi);
}
// if writing only 1 pixel, send 16-bit color spi messages
// if writing the whole screen, use DMA
// ramwr instruction

// command parser
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
	data = 0xC0;
	sendCommand(ST77XX_MADCTL, &data, 1, hspi);
}

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

void SPI_CS_LOW() {
	HAL_GPIO_WritePin(CS_GPIO, CS_PIN, GPIO_PIN_RESET);
}

void SPI_CS_HIGH() {
	HAL_GPIO_WritePin(CS_GPIO, CS_PIN, GPIO_PIN_SET);
}

void SPI_DC_LOW() {
	HAL_GPIO_WritePin(DC_GPIO, DC_PIN, GPIO_PIN_RESET);
}

void SPI_DC_HIGH() {
	HAL_GPIO_WritePin(DC_GPIO, DC_PIN, GPIO_PIN_SET);
}

void TFT_startup(SPI_HandleTypeDef *hspi) {
	displayInit(Rcmd1, hspi);
	displayInit(Rcmd2red, hspi);
	displayInit(Rcmd3, hspi);
	setAddrWindow(0, 0, 128, 160, hspi);
//	setAddrWindow(0, 0, 8, 8, hspi);
//	sendCommand16(0xAA, 0xAA, 1, &hspi1);
}

void fillScreen(SPI_HandleTypeDef *hspi) {
	setAddrWindow(0, 0, WIDTH, HEIGHT/2, hspi);

	// cant do in 1 push for size reasons? too many elements in array for size
	uint16_t arr[WIDTH*HEIGHT/2];
	for (int i = 0; i < (WIDTH*HEIGHT/2); i++) {
		arr[i] = ST77XX_BLACK;
	}
	sendCommand(ST77XX_RAMWR, arr, WIDTH*HEIGHT, hspi);
	setAddrWindow(0, HEIGHT/2, WIDTH, HEIGHT/2, hspi);
	sendCommand(ST77XX_RAMWR, arr, WIDTH*HEIGHT, hspi);

//	uint16_t arr[10240];
//	// cant do in 1 push for size reasons?
//	// too many elements in array for size
//	for (int i = 0; i < (10240); i++) {
//		arr[i] = ST77XX_CYAN;
//	}
//	sendCommand(ST77XX_RAMWR, arr, 20480, hspi);
//
//	setAddrWindow(0, height/2, width, height/2, hspi);
//	sendCommand(ST77XX_RAMWR, arr, 20480, hspi);
}

void fillScreen_color(uint16_t color, SPI_HandleTypeDef *hspi) {
	setAddrWindow(0, 0, WIDTH, HEIGHT, hspi);
	uint16_t arr[WIDTH*HEIGHT];
	for (int i = 0; i < WIDTH*HEIGHT; i++) {
		arr[i] = color;
	}
	sendCommand(ST77XX_RAMWR, arr, WIDTH*HEIGHT*2, hspi);
}

void drawPixel(uint8_t x, uint8_t y, SPI_HandleTypeDef *hspi) {
	setAddrWindow(x, y, 1, 1, hspi);
	uint16_t color = ST77XX_BLACK;
	sendCommand(ST77XX_RAMWR, &color, 2, hspi);
}

void drawPixel_color(uint8_t x, uint8_t y, uint16_t color, SPI_HandleTypeDef *hspi) {
	setAddrWindow(x, y, 1, 1, hspi);
	uint16_t tempColor = color;		// else we're using address of something passed by value (problems?)
	sendCommand(ST77XX_RAMWR, &tempColor, 2, hspi);
}

void drawHLine(uint8_t x, uint8_t y, uint8_t size, SPI_HandleTypeDef *hspi) {
	setAddrWindow(x, y, size, 1, hspi);
	uint16_t color[size];
	for (int i = 0; i < size; i++) {		// better way to make array of 1 color; SPI without moving address of sent?
		color[i] = ST77XX_BLACK;
	}

	sendCommand(ST77XX_RAMWR, color, size*2, hspi);
}

void drawHLine_color(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef *hspi) {
	setAddrWindow(x, y, size, 1, hspi);
	uint16_t colors[size];
	for (int i = 0; i < size; i++) {		// better way to make array of 1 color; SPI without moving address of sent?
		colors[i] = color;
	}

	sendCommand(ST77XX_RAMWR, colors, size*2, hspi);
}

void drawVLine(uint8_t x, uint8_t y, uint8_t size, SPI_HandleTypeDef *hspi) {
	setAddrWindow(x, y, 1, size, hspi);
	uint16_t color[size];
	for (int i = 0; i < size; i++) {		// better way to make array of 1 color; SPI without moving address of sent?
		color[i] = ST77XX_BLACK;
	}

	sendCommand(ST77XX_RAMWR, color, size*2, hspi);
}

void drawVLine_color(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef *hspi) {
	setAddrWindow(x, y, 1, size, hspi);
	uint16_t colors[size];
	for (int i = 0; i < size; i++) {		// there has to be a better way to make array of 1 color
											// SPI without moving address of sent buffer?
		colors[i] = color;
	}

	sendCommand(ST77XX_RAMWR, colors, size*2, hspi);
}

void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, SPI_HandleTypeDef *hspi) {
	for (int i = 0; i < h; i++) {
		drawHLine_color(x, y, w, colorFixer(ST77XX_BLACK), hspi);
	}
}

void drawRect_color(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef *hspi) {
	for (int i = 0; i < h; i++) {
		drawHLine_color(x, y, w, colorFixer(color), hspi);
	}
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
