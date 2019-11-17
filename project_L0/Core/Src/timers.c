// file for using hardware timers for various things
// who knows if im doing this right lmao

// should set flags to update screen? hmmmmmmm....

#include "timers.h"
#include "clocks.h"

// ---- Stopwatch functions ----
// set stopwatch. using lptimer. maybe better with regular timer?
// can operate in stop mode if using lptimer
// modify to update screen/set flags when necessary
void runStopwatch(LPTIM_HandleTypeDef *hlptim) {
	HAL_LPTIM_Counter_Start_IT(hlptim, 0x8000);
}

// stop the timer or pause it or whatever.
// counter value might reset and screw up timekeeping? should save?
void pauseStopwatch(LPTIM_HandleTypeDef *hlptim) {
	HAL_LPTIM_Counter_Stop_IT(hlptim);
//	temp = hlptim->Instance->CNT;
}

void clearStopwatch(LPTIM_HandleTypeDef *hlptim) {
	pauseStopwatch(hlptim);
	stopwatchCNT = 0;
}

// increment variable for stopwatch counting.
// update screen if on
// how to set lptim internal clock to LSE???
//   I FOUND IT: RCC->CCIPR LPTIMSEL (2-bits, 11=LSE clock for LPTIM)
//   now...does hal do this automatically?
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim) {
	// toggle pin, should toggle every 1s. change this pin
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
	stopwatchCNT++;
}
// ---- end of stopwatch functions ----


// ---- timer functions (like the timer functionality, not hardware timer) ----
// used for screen updates.
// else, we're setting rtc alarm
// uses TIM21 with LSE (external timer w/ remap and done already by ST).
//void setTimer(TIM_HandleTypeDef *htim, struct alarmTimes *a) {
//	HAL_TIM_Base_Start_IT(htim);
//}
void runTimerDisplay(TIM_HandleTypeDef *htim) {
	HAL_TIM_Base_Start(htim);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// should toggle pin every 1s. change pin
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
	if (watchTimerSeconds != 0) watchTimerSeconds--;
}
// ---- end of timer functions ----


// ---- motor/things that use PWM functions ----
// running motor for vibration. should run for a finite time
// connect motor enable line to timer output line
void runMotor(TIM_HandleTypeDef *htim) {
	HAL_TIM_OnePulse_Start_IT(htim, HAL_TIM_ACTIVE_CHANNEL_1);
}
// ---- end of motor (and other) functions ----
