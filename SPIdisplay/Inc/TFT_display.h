#include "stm32l4xx_hal.h"

// from st7735 file in Adafruit github
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

// from st77xx file in Adafruit github
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

// Some ready-made 16-bit ('565') color settings:
#define	ST77XX_BLACK      0x0000
#define ST77XX_WHITE      0xFFFF
#define	ST77XX_RED        0xF800
#define	ST77XX_GREEN      0x07E0
#define	ST77XX_BLUE       0x001F
#define ST77XX_CYAN       0x07FF
#define ST77XX_MAGENTA    0xF81F
#define ST77XX_YELLOW     0xFFE0
#define	ST77XX_ORANGE     0xFC00

// my own defines
#define CS_PIN		GPIO_PIN_12
#define CS_GPIO		GPIOE
#define DC_PIN		GPIO_PIN_11
#define DC_GPIO		GPIOE
#define _xstart		0
#define _ystart 	0

#define HEIGHT 		160
#define WIDTH 		128

#define swap_ints(x,y) x=x+y; y=x-y; x=x-y
#define abs(x) ((x) > 0 ? (x) : -(x))

void SPI_CS_LOW();
void SPI_CS_HIGH();
void SPI_DC_LOW();
void SPI_DC_HIGH();
void sendCommand(uint8_t, uint8_t*, uint16_t, SPI_HandleTypeDef*);
void displayInit(uint8_t*, SPI_HandleTypeDef*);
//uint32_t toBigEndian_32(uint32_t);
void TFT_startup(SPI_HandleTypeDef*);
void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SPI_HandleTypeDef *hspi);

uint16_t colorFixer(uint16_t);
void drawPixel(uint8_t x, uint8_t y, uint16_t color, SPI_HandleTypeDef* hspi);
void drawHLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef* hspi);
void drawVLine(uint8_t x, uint8_t y, uint8_t size, uint16_t color, SPI_HandleTypeDef* hspi);
void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color, SPI_HandleTypeDef* hspi);
void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color, SPI_HandleTypeDef* hspi);
void fillScreen(uint16_t color, SPI_HandleTypeDef* hspi);
