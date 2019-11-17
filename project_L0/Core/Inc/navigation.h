/*
 * file for handling navigation between different apps/interfaces.
 * uses global flags (concurrency problem?). Displays information
 * using functions defined in TFT_display.c.
 */

#include "stm32l0xx_hal.h"
#include "TFT_display.h"

// uses gpio external interrupts. interrupt handler/callback function
// ignores ports, so only pin number is needed.
#define BUTTON0 GPIO_PIN_2
#define BUTTON1 GPIO_PIN_13
#define BUTTON2 GPIO_PIN_14
#define BUTTON3 GPIO_PIN_15

// defines to help with flags
#define NUM_FACES 4
#define NUM_CLOCKFIELDS 5		// min, hr, year, month, day (year, month first for getting bounds of day)
#define NUM_TIMERFIELDS 3		// sec, min, hr
#define NUM_ALARMFIELDS 4		// sec, min, hr, day of week
#define NUM_STOPWATCHFIELDS 4	// ms, sec, min, hr

// flag idea: software toggle a pin connected to edge detector to trigger
// hardware interrupt from software lmao

// variables to track state of apps, set in callback/interrupt
// straight global instead of static/scope in file only?
// need volatile keyword?

static int faceChange;		// flag set and cleared in different places, careful

// ---- buncha flags ----
volatile uint8_t updateFace;
volatile uint8_t updateClock;
volatile uint8_t updateTimer;
volatile uint8_t updateAlarm;
volatile uint8_t updateStopwatch;
// ---- end of flags ----

// for face on screen
static int face;

// for clock
static int clockSet;
static int clockField;
struct dates tempClockDate;
struct times tempClockTimes;

// for timer
static int timerSet;
static int timerField;
static int timerRunning;
struct times tempTimer;

// for alarm
static volatile int alarmSet;
static int alarmField;
static int alarmRunning;
struct alarmTimes tempAlarm;

// for stopwatch
static int stopwatchRunning;
int tempStopwatch[NUM_STOPWATCHFIELDS];

// using volatile keyword
//static volatile int face;
//static volatile int faceChange;		// flag set and cleared in different places, careful
//
//// for clock
//static volatile int clockSet;
//static volatile int clockField;
//int tempClock[NUM_CLOCKFIELDS];
//
//// for timer
//static volatile int timerSet;
//static volatile int timerField;
//static volatile int timerRunning;
//int tempTimer[NUM_TIMERFIELDS];
//
//// for alarm
//static volatile int alarmSet;
//static volatile int alarmField;
//static volatile int alarmRunning;
//int tempAlarm[NUM_ALARMFIELDS];
//
//// for stopwatch
//static volatile int stopwatchRunning;
//int tempStopwatch[NUM_STOPWATCHFIELDS];

// enum for different faces used (clock, timer, alarm, stopwatch)
// probably not needed
enum displayFaces {
	faceClock,
	faceTimer,
	faceAlarm,
	faceStopwatch
};

const unsigned char weekdayNames[8] = {
	"",			// padding so i can avoid dealing with out-of-bounds access
	"mon  ",
	"tues ",
	"wed  ",
	"thurs",
	"fri  ",
	"sat  ",
	"sun  "
};

const unsigned char monthNames[12] = {
	"jan ",
	"feb ",
	"mar ",
	"apr ",
	"may ",
	"jun ",
	"jul ",
	"aug ",
	"sept",
	"oct ",
	"nov ",
	"dec "
};

void updateDisplay(SPI_HandleTypeDef *hspi);
