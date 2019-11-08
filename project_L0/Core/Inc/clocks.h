// header file for using RTC and timers.

#ifndef CLOCKS_H
#define CLOCKS_H

#include "stm32l0xx_hal.h"
#include "TFT_display.h"
#include <stdio.h>

/*
 * l0 pinout:
 * spi: pa5 = sck, pa7 = mosi
 * buttons: pb2, pb13, pb14, pb15
 */

// uncomment after you get functions args sorted
void setTime(RTC_HandleTypeDef *hrtc);
void setDate(RTC_HandleTypeDef *hrtc);
void setDateTime(RTC_HandleTypeDef *hrtc);
//
//void setAlarm();
//
void printTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void printDate(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void printDateTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);

void clockTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);

#endif
