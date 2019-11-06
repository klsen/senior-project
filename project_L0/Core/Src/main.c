/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "navigation.h"
#include "TFT_display.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void lineTest(uint16_t bg) {
	int x = WIDTH/2;
	int y = HEIGHT/2;

	drawLine(x, y, x+WIDTH/2, y+HEIGHT/4, ST77XX_RED, &hspi1);
	HAL_Delay(500);
	drawRect(x, y, WIDTH/2, HEIGHT/4, ST77XX_RED, &hspi1);
	HAL_Delay(1000);

	drawLine(x, y, x+WIDTH/4, y+HEIGHT/2, ST77XX_BLUE, &hspi1);
	HAL_Delay(500);
	drawRect(x, y, WIDTH/4, HEIGHT/2, ST77XX_BLUE, &hspi1);
	HAL_Delay(1000);

	drawLine(x, y, x-WIDTH/4, y+HEIGHT/2, ST77XX_YELLOW, &hspi1);
	HAL_Delay(500);
	drawRect(x-WIDTH/4, y, WIDTH/4, HEIGHT/2, ST77XX_YELLOW, &hspi1);
	HAL_Delay(1000);

	drawLine(x, y, x-WIDTH/2, y+HEIGHT/4, ST77XX_GREEN, &hspi1);
	HAL_Delay(500);
	drawRect(x-WIDTH/2, y, WIDTH/2, HEIGHT/4, ST77XX_GREEN, &hspi1);
	HAL_Delay(1000);

	drawLine(x, y, x-WIDTH/2, y-HEIGHT/4, ST77XX_ORANGE, &hspi1);
	HAL_Delay(500);
	drawRect(x-WIDTH/2, y-HEIGHT/4, WIDTH/2, HEIGHT/4, ST77XX_ORANGE, &hspi1);
	HAL_Delay(1000);

	drawLine(x, y, x-WIDTH/4, y-HEIGHT/2, ST77XX_MAGENTA, &hspi1);
	HAL_Delay(500);
	drawRect(x-WIDTH/4, y-HEIGHT/2, WIDTH/4, HEIGHT/2, ST77XX_MAGENTA, &hspi1);
	HAL_Delay(1000);

	drawLine(x, y, x+WIDTH/4, y-HEIGHT/2, ST77XX_CYAN, &hspi1);
	HAL_Delay(500);
	drawRect(x, y-HEIGHT/2, WIDTH/4, HEIGHT/2, ST77XX_CYAN, &hspi1);
	HAL_Delay(1000);

	drawLine(x, y, x+WIDTH/2, y-HEIGHT/4, ST77XX_WHITE, &hspi1);
	HAL_Delay(500);
	drawRect(x, y-HEIGHT/4, WIDTH/2, HEIGHT/4, ST77XX_WHITE, &hspi1);
	HAL_Delay(1000);

	fillScreen(bg, &hspi1);
}

void charTest(uint16_t bg) {
	uint8_t x, y;

	uint16_t rainbowColors[] = {
		ST77XX_RED,
		ST77XX_ORANGE,
		ST77XX_YELLOW,
		ST77XX_GREEN,
		ST77XX_CYAN,
		ST77XX_BLUE,
		ST77XX_MAGENTA
	};

	// print the standard 127 6x8 characters in different sizes
	for (int ch_size = 1; ch_size < 6; ch_size++) {
		for (unsigned char ch = 0; ch < 255; ch++) {
			// move to right enough for next char
			x = (ch*ch_size*6) % (WIDTH/(ch_size*6)*(ch_size*6));
			// line break when x gets near WIDTH
			y = ((8*ch_size) * ((ch*ch_size*6) / (WIDTH/(ch_size*6)*(ch_size*6)))) % (HEIGHT/(ch_size*8)*(ch_size*8));

			drawChar(x, y, ch, rainbowColors[ch%7], bg, ch_size, ch_size, &hspi1);
//			HAL_Delay(250);
		}
		HAL_Delay(1000);
		fillScreen(bg, &hspi1);
	}
}

void textTest(uint16_t bg) {
	char *str = "Hello";
	drawText(0, 0, 1, ST77XX_WHITE, str, &hspi1);
	HAL_Delay(1000);

	str = "Looooooooooooooooooooooooooooooong string";
	drawText(0, 16, 1, ST77XX_WHITE, str, &hspi1);
	HAL_Delay(1000);

	str = "more text";
	drawText(0, 40, 1, ST77XX_WHITE, str, &hspi1);
	HAL_Delay(1000);

	str = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	drawText(0, 64, 1, ST77XX_WHITE, str, &hspi1);
	HAL_Delay(500);

	fillScreen(bg, &hspi1);
}

void basicTest(uint16_t bg) {
	drawPixel(0, 0, ST77XX_BLUE, &hspi1);
	HAL_Delay(250);
	drawHLine(0, 159, 128, ST77XX_RED, &hspi1);
	HAL_Delay(250);
	drawVLine(127, 0, 160, ST77XX_GREEN, &hspi1);
	drawRect(50, 50, 50, 50, ST77XX_CYAN, &hspi1);

	HAL_Delay(500);

	fillScreen(bg, &hspi1);
	drawPixel(0, 0, bg, &hspi1);
	drawHLine(0, 159, 128, bg, &hspi1);
	drawVLine(127, 0, 160, bg, &hspi1);
	drawRect(50, 50, 50, 50, bg, &hspi1);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_ADC_Init();
  /* USER CODE BEGIN 2 */
	uint16_t bg = ST77XX_BLACK;
	HAL_Delay(2000);
	TFT_startup(&hspi1);
	fillScreen(bg, &hspi1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
	  HAL_Delay(1000);
//	  lineTest(bg);
//	  charTest(bg);
//	  textTest(bg);
//	  updateDisplay(&hspi1);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc.Instance = ADC1;
  hadc.Init.OversamplingMode = DISABLE;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerFrequencyMode = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted. 
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB1 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB13 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
