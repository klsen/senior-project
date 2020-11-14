#ifndef INPUT_TEST_H
#define INPUT_TEST_H

#include "stm32l0xx_hal.h"
#include "TFT_display.h"
#include "user_interface.h"

void input_test(SPI_HandleTypeDef *hspi);
void layout(SPI_HandleTypeDef *hspi);
void buttonTappedVisual(uint16_t boxX, uint16_t boxY, uint8_t isVisible, SPI_HandleTypeDef *hspi);
void buttonHeldVisual(uint16_t boxX, uint16_t boxY, uint8_t isVisible, SPI_HandleTypeDef *hspi);

#endif
