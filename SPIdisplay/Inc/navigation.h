// rename to a more appropriate name

#include "stm32l4xx_hal.h"
#include "main.h"
#include "TFT_display.h"

#define BUTTON0 = GPIO_PIN_0
#define BUTTON1 = GPIO_PIN_1
#define BUTTON2 = GPIO_PIN_2
#define BUTTON3 = GPIO_PIN_3

#define NUM_FACES 4
#define NUM_CLOCKFIELDS 5		// min, hr, year, month, day (year, month first for getting bounds of day)
#define NUM_TIMERFIELDS 3		// sec, min, hr
#define NUM_ALARMFIELDS 4		// sec, min, hr, day of week
#define NUM_STOPWATCHFIELDS 4	// ms, sec, min, hr

// flag idea: software toggle a pin connected to edge detector to trigger
// hardware interrupt from software lmao

// variables to track state of apps, set in callback/interrupt
// straight global instead of static (scope in file only)?
// need volatile keyword?
static int face;
static int faceChange;		// flag set and cleared in different places, careful

// for clock
static int clockSet;
static int clockField;
int tempClock[NUM_CLOCKFIELDS];

// for timer
static int timerSet;
static int timerField;
static int timerRunning;
int tempTimer[NUM_TIMERFIELDS];

// for alarm
static int alarmSet;
static int alarmField;
static int alarmRunning;
int tempAlarm[NUM_ALARMFIELDS];

// for stopwatch
static int stopwatchRunning;
int tempStopwatch[NUM_STOPWATCHFIELDS];

// enum for different faces used (clock, timer, alarm, stopwatch)
// probably not needed
enum displayFaces {
	faceMain,
	faceTimer,
	faceAlarm,
	faceStopwatch
};

void updateDisplay();
