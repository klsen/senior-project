/*
 * Tests graphics functions for the Adafruit TFTLCD display,
 * which uses the ST7735R driver chip and communicates using SPI.
 */

// draws diagonal, horizontal, and vertical lines
void lineTest(uint16_t bg, SPI_HandleTypeDef *hspi);

// draws all characters in the character set in different colors
// and different sizes
void charTest(uint16_t bg, SPI_HandleTypeDef *hspi);

// draws small bits of text using a char*
void textTest(uint16_t bg, SPI_HandleTypeDef *hspi);

// draws pixels, horizontal lines, and vertical lines
void basicTest(uint16_t bg, SPI_HandleTypeDef *hspi);
