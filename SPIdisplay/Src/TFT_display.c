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

	HAL_Delay(500);

	//writeCommand(ST77XX_RAMWR); // write to RAM
//	uint16_t colors[8] = {
//		ST77XX_BLACK,
//		ST77XX_BLUE,
//		ST77XX_GREEN,
//		ST77XX_BLACK,
//		ST77XX_RED,
//		ST77XX_YELLOW,
//		ST77XX_ORANGE,
//		ST77XX_MAGENTA
//	};
	uint16_t colors[10000];
	for (int i = 0; i < 10000; i++) {
		colors[i] = 0xF0;
	}
	sendCommand(ST77XX_RAMWR, colors, 20000, hspi);
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

//void sendCommand16(uint8_t cmd, uint16_t *args, uint8_t numArgs, SPI_HandleTypeDef *hspi) {
//	SPI_CS_LOW();	// chip select
//
//	SPI_DC_LOW();	// command mode
//	HAL_SPI_Transmit(hspi, &cmd, 1, 1000);
//
//	SPI_DC_HIGH();	// data mode
//	HAL_SPI_Transmit(hspi, &args, numArgs, 1000);
//
//	SPI_CS_HIGH();	// chip select disable
//}

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
	setAddrWindow(0, 0, 120, 120, hspi);
//	setAddrWindow(0, 0, 8, 8, hspi);
//	sendCommand16(0xAA, 0xAA, 1, &hspi1);
}
