// file for using hardware timers for various things
// who knows if im doing this right lmao

// should set flags to update screen? hmmmmmmm....

#include "timers.h"

// static variables
static uint32_t timerStartMarker = 0;
static uint32_t timerPauseMarker = 0;
static uint32_t stopwatchStartMarker = 0;
static uint32_t stopwatchPauseMarker = 0;
static uint8_t motorStateCounter = 0;

// important boye that is called for a bunch of different timers
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// button's timer
	if (htim->Instance == TIM6) {
		// renable button interrupts and clear pending
		HAL_TIM_Base_Stop_IT(htim);
		HAL_NVIC_ClearPendingIRQ(EXTI2_3_IRQn);
		HAL_NVIC_ClearPendingIRQ(EXTI4_15_IRQn);
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON1);
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON2);
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON3);
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON4);

		HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	}
	// sampler's timer
	else if (htim->Instance == TIM22) {
		canSampleBattery = 1;
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM21) {
		// timer's channel
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			updateFace.timer = 1;
			if (timerCounter != 1) --timerCounter;
			else {
				--timerCounter;
				isTimerDone = 1;
				stopTimer(htim);
				updateFace.timer = 1;

				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
			}
		}
		// stopwatch's channel
		else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
			updateFace.stopwatch = 1;
			++stopwatchCounter;
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
		}
	}
	// motor's timer
	else if (htim->Instance == TIM2) {
		// just pulsing 3x
		++motorStateCounter;
		switch(motorStateCounter) {
			case 1: HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_PIN, GPIO_PIN_SET); break;
			case 2: HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_PIN, GPIO_PIN_RESET); break;
			case 3: HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_PIN, GPIO_PIN_SET); break;
			case 4: HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_PIN, GPIO_PIN_RESET); break;
			case 5: HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_PIN, GPIO_PIN_SET); break;
			case 6:
				HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_PIN, GPIO_PIN_RESET); break;
				stopMotor(htim);
				break;
			default: break;
		}
	}
}

// ---- important timer functions  ----
void runTimer(TIM_HandleTypeDef *htim) {
	TIM_OC_InitTypeDef sConfig = {0};
	sConfig.OCMode = TIM_OCMODE_TIMING;
	sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode = TIM_OCFAST_DISABLE;

	// calculating pulse/OC trigger
	if (isTimerPaused == 0) {		// 1st run, hasn't been paused yet
		sConfig.Pulse = htim->Instance->CNT;
	}
	else {		// unpausing
		uint32_t temp = htim->Instance->CNT;

		// count needed to get from pause marker to start marker - basically how many steps it would've taken to get to next cycle
		// shifted to account for negative behavior
		uint32_t diff = ((int)(timerStartMarker-timerPauseMarker)+0x8000) % 0x8000;
		sConfig.Pulse = (temp+diff) % 0x8000;
		timerStartMarker = sConfig.Pulse;		// set new start marker
	}

	HAL_TIM_OC_ConfigChannel(htim, &sConfig, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(htim, TIM_CHANNEL_1);
}

// flags should be set in nav
void pauseTimer(TIM_HandleTypeDef *htim) {
	HAL_TIM_OC_Stop_IT(htim, TIM_CHANNEL_1);
	timerPauseMarker = htim->Instance->CNT;
}

void stopTimer(TIM_HandleTypeDef *htim) {
	HAL_TIM_OC_Stop_IT(htim, TIM_CHANNEL_1);
	timerStartMarker = 0;
	timerPauseMarker = 0;
}

void runStopwatch(TIM_HandleTypeDef *htim) {
	TIM_OC_InitTypeDef sConfig = {0};
	sConfig.OCMode = TIM_OCMODE_TIMING;
	sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode = TIM_OCFAST_DISABLE;

	// calculating pulse/OC trigger
	if (isStopwatchPaused == 0) {		// 1st run, hasn't been paused yet
		stopwatchCounter = 0;
		sConfig.Pulse = htim->Instance->CNT;
		stopwatchStartMarker = sConfig.Pulse;		// set new start marker
	}
	else {		// unpausing
		uint32_t temp = htim->Instance->CNT;

		// count needed to get from pause marker to start marker - basically how many steps it would've taken to get to next cycle
		// shifted to account for negative behavior
		uint32_t diff = ((int)(stopwatchStartMarker-stopwatchPauseMarker)+0x8000) % 0x8000;
		sConfig.Pulse = (temp+diff) % 0x8000;
		stopwatchStartMarker = sConfig.Pulse;		// set new start marker
	}

	HAL_TIM_OC_ConfigChannel(htim, &sConfig, TIM_CHANNEL_2);
	HAL_TIM_OC_Start_IT(htim, TIM_CHANNEL_2);
}

// stop the timer or pause it or whatever.
// counter value might reset and screw up timekeeping? should save?
void pauseStopwatch(TIM_HandleTypeDef *htim) {
	HAL_TIM_OC_Stop_IT(htim, TIM_CHANNEL_2);
	stopwatchPauseMarker = htim->Instance->CNT;
}

void clearStopwatch(TIM_HandleTypeDef *htim) {
	HAL_TIM_OC_Stop_IT(htim, TIM_CHANNEL_2);
	stopwatchPauseMarker = htim->Instance->CNT;
	stopwatchStartMarker = htim->Instance->CNT;

	stopwatchCounter = 0;
}

void runTimerStopwatchBase(TIM_HandleTypeDef *htim) {HAL_TIM_Base_Start(htim);}
void stopTimerStopwatchBase(TIM_HandleTypeDef *htim) {HAL_TIM_Base_Stop(htim);}
// ---- end of important timer functions ----

// ---- motor and other things that use timer ----
// uses LSE timer TIM22
void runADCSampler(TIM_HandleTypeDef *htim) {
	HAL_TIM_Base_Start_IT(htim);
}

// uses LSE timer TIM2 CH1
void runDisplayBacklight(TIM_HandleTypeDef *htim) {
	TIM_OC_InitTypeDef sConfig = {0};
	sConfig.OCMode = TIM_OCMODE_PWM1;
	sConfig.Pulse = htim->Instance->ARR-1;
	sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode = TIM_OCFAST_DISABLE;

	HAL_TIM_PWM_ConfigChannel(htim, &sConfig, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start_IT(htim, TIM_CHANNEL_1);
}

// should change display brightness by changing PWM pulse width. input should be from 0-100
void changeDisplayBacklight(uint8_t intensity, TIM_HandleTypeDef *htim) {
	if (intensity > 100) return;		// bounds checking

	TIM_OC_InitTypeDef sConfig = {0};
	sConfig.OCMode = TIM_OCMODE_PWM1;
	sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode = TIM_OCFAST_DISABLE;
	sConfig.Pulse = htim->Instance->ARR-1 * (float)intensity/100;

	HAL_TIM_PWM_ConfigChannel(htim, &sConfig, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start_IT(htim, TIM_CHANNEL_1);
}

void stopDisplayBacklight(TIM_HandleTypeDef *htim) {
	HAL_TIM_PWM_Stop_IT(htim, TIM_CHANNEL_1);
}

// running motor for vibration. should run for a finite time
// connect motor enable line to timer output line
// uses LSE timer TIM2 CH2
void runMotor(TIM_HandleTypeDef *htim) {
	TIM_OC_InitTypeDef sConfig = {0};
	sConfig.OCMode = TIM_OCMODE_TIMING;
	sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode = TIM_OCFAST_DISABLE;
	sConfig.Pulse = htim->Instance->CNT;

	HAL_GPIO_WritePin(MOTOR_PORT, MOTOR_PIN, GPIO_PIN_SET);
	HAL_TIM_OC_ConfigChannel(htim, &sConfig, TIM_CHANNEL_2);
	HAL_TIM_OC_Start_IT(htim, TIM_CHANNEL_2);

	motorStateCounter = 0;
}

void stopMotor(TIM_HandleTypeDef *htim) {
	// set flag only? to start looking at period complete interrupt for motor's timer?
	HAL_TIM_OC_Stop_IT(htim, TIM_CHANNEL_2);
	motorStateCounter = 0;
}

// should use TIM22
void runBacklightMotorBase(TIM_HandleTypeDef *htim) {HAL_TIM_Base_Start(htim);}
void stopBacklightMotorBase(TIM_HandleTypeDef *htim) {HAL_TIM_Base_Stop(htim);}
// ---- end of motor (and other) functions ----
