/*
 * test file for clock.c and timers.c files.
 */

#ifndef CLOCK_TIMER_TEST_H
#define CLOCK_TIMER_TEST_H

#include "clocks.h"
#include "timers.h"
#include "TFT_display.h"
#include <stdio.h>		// for sprintf

void printTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void printDate(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void printDateTime(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);

void clockTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
void alarmTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);
//void timerTest(RTC_HandleTypeDef *hrtc, SPI_HandleTypeDef *hspi);

//void clockTest();
//void alarmTest();
//void timerTest();

#endif
