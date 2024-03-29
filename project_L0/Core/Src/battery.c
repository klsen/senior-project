// Implementation file for battery.h

#include "battery.h"

static const float batteryCapacity[];
static uint16_t batteryCapacityArraySize = 179;

// spi used to turn display on/off and drawing battery graphic
void batteryManager(ADC_HandleTypeDef *hadc, SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *backlightTim) {
	if (canSampleBattery) {
		canSampleBattery = 0;

		battPercentage = getBatteryPercentage(hadc);

		// start really shutting down & set flag
		// disable power supply (setting enable pin to 0)
		if (battPercentage == 0) {
			// power off supply
			turnDisplayOff(hspi);
			HAL_GPIO_WritePin(POWER_SUPPLY_ENABLE_PORT, POWER_SUPPLY_ENABLE_PIN, GPIO_PIN_RESET);
		}
		else if (battPercentage <= 5) {
			// start turning off hardware
			turnDisplayOff(hspi);
			startLowPowerMode(timerStopwatchTim, backlightTim);
			bState = batteryReallyLow;
		}
		// start low-power mode and set flag
		else if (battPercentage <= 15) {
			// start turning off hardware
			bState = batteryLow;
		}
		// set hardware to use power normally
		else {
			if (bState == batteryLow || bState == batteryReallyLow) {
				turnDisplayOn(hspi);
				stopLowPowerMode(timerStopwatchTim, backlightTim);
			}
			HAL_GPIO_WritePin(POWER_SUPPLY_ENABLE_PORT, POWER_SUPPLY_ENABLE_PIN, GPIO_PIN_SET);
			bState = batteryNormal;
		}

		drawBattery(battPercentage, hspi);
	}
}

void startLowPowerMode(TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *backlightTim) {
	setDisplayBacklight(50, backlightTim);
	stopTimerStopwatchBase(timerStopwatchTim);
}

void stopLowPowerMode(TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *backlightTim) {
	setDisplayBacklight(100, backlightTim);
	runTimerStopwatchBase(timerStopwatchTim);
}

// should return a number from 0-100
uint8_t getBatteryPercentage(ADC_HandleTypeDef *hadc) {
	float v, temp;
	uint8_t index;

	// enable adc voltage divider for measurements, disable after
	HAL_GPIO_WritePin(ADC_DIVIDER_PORT, ADC_DIVIDER_PIN, GPIO_PIN_SET);
	HAL_ADC_Start_IT(hadc);

	HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
	v = 3.3*HAL_ADC_GetValue(hadc)/(0xFFF);

	HAL_ADC_Stop(hadc);
	HAL_GPIO_WritePin(ADC_DIVIDER_PORT, ADC_DIVIDER_PIN, GPIO_PIN_RESET);

	// trying to look only for 3.9-3.4. anything above 3.7 is 100%, anything below 3.4 is 0%
	// scaled voltages at 3.0642-2.6714
	// indices at 6-151. have to scale and flip to go from 100-0 since 6->100%
	index = search(v);
	if (index <= 6) return 100;
	else if (index >= 135) return 0;		// adjusting numbers because tests want to call 2.65V non-zero
	else {
		index -= 6;
		temp = index*100.0/129;
		temp = 100-temp;
		return temp;
	}
}

// should return index in array
uint8_t search(float val) {
	// O(n) lookup. array is only size=179.
	uint8_t i;
	for (i = 0; i < batteryCapacityArraySize; i++) {
		if (val > batteryCapacity[i]) return i;
	}
	return batteryCapacityArraySize;
}

// small tester for battery functions
void testBatteryCalculator(ADC_HandleTypeDef *hadc, SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *timerStopwatchTim, TIM_HandleTypeDef *backlightTim) {
	batteryManager(hadc, hspi, timerStopwatchTim, backlightTim);

	setTextSize(1);
	setTextColor(ST77XX_ORANGE);
	setBackgroundColor(ST77XX_BLACK);
	char str[40];
	sprintf(str, "batt_level: %3d %%", battPercentage);
	drawTextAt(0, 0, str, hspi);
}

// from some data set hosted hosted by nasa. should be replaced later with measurements from our own battery
// thx for the data Prognostics CoE at NASA Ames
// already scaled for 3.3V ADC. sorted by largest to smallest, so should do a conversion when indexing
// only looking at a certain section when curve looks mostly linear
static const float batteryCapacity[] = {
	3.293315,
	3.292731,
	3.123113,
	3.104920,
	3.091277,
	3.080046,
	3.070496,
	3.061957,
	3.054446,
	3.047753,
	3.041513,
	3.035591,
	3.030233,
	3.025011,
	3.020161,
	3.015326,
	3.010809,
	3.006468,
	3.002290,
	2.998205,
	2.994119,
	2.990301,
	2.986596,
	2.982802,
	2.978996,
	2.975205,
	2.971621,
	2.967781,
	2.964034,
	2.960522,
	2.956823,
	2.953116,
	2.949739,
	2.946137,
	2.942633,
	2.939361,
	2.935978,
	2.932665,
	2.929420,
	2.926419,
	2.923369,
	2.920036,
	2.917194,
	2.913765,
	2.910908,
	2.907679,
	2.904595,
	2.901674,
	2.898639,
	2.895526,
	2.892602,
	2.889694,
	2.886763,
	2.883809,
	2.881005,
	2.878070,
	2.875451,
	2.872488,
	2.869685,
	2.866886,
	2.864358,
	2.861631,
	2.858793,
	2.856131,
	2.853451,
	2.851026,
	2.848224,
	2.845529,
	2.843077,
	2.840410,
	2.837907,
	2.835312,
	2.832920,
	2.830289,
	2.827673,
	2.825246,
	2.823085,
	2.820487,
	2.818084,
	2.815562,
	2.813238,
	2.810697,
	2.808433,
	2.806010,
	2.803995,
	2.801507,
	2.799182,
	2.796974,
	2.794702,
	2.792408,
	2.790227,
	2.788119,
	2.785990,
	2.783873,
	2.781659,
	2.779387,
	2.777647,
	2.775543,
	2.773495,
	2.771644,
	2.769655,
	2.767760,
	2.765803,
	2.764118,
	2.762089,
	2.760437,
	2.758753,
	2.756918,
	2.755414,
	2.753702,
	2.752189,
	2.750504,
	2.749089,
	2.747258,
	2.745884,
	2.744224,
	2.742853,
	2.741238,
	2.739683,
	2.738211,
	2.736779,
	2.735069,
	2.733758,
	2.732096,
	2.730601,
	2.729134,
	2.727362,
	2.725767,
	2.724196,
	2.722577,
	2.721084,
	2.719173,
	2.717416,
	2.715724,
	2.713717,
	2.711964,
	2.710198,
	2.708288,
	2.706265,
	2.704319,
	2.702024,
	2.699998,
	2.697884,
	2.695704,
	2.693288,
	2.690949,
	2.688524,
	2.686158,
	2.683574,
	2.680681,
	2.677945,
	2.674853,
	2.671614,
	2.668388,
	2.664996,
	2.661252,
	2.657310,
	2.653002,
	2.648212,
	2.642976,
	2.637527,
	2.631104,
	2.623964,
	2.616142,
	2.607163,
	2.596740,
	2.585297,
	2.572205,
	2.557120,
	2.540295,
	2.520790,
	2.498541,
	2.472818,
	2.443045,
	2.408514,
	2.367405,
	2.317233,
	2.251871,
	2.166412
};
