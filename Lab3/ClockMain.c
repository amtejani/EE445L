// ******** ClockMain.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/8/2017
// Main to test modules for clock
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/9/2017

#define ADC_READ_HOUR			((BaseTime/100 + ADCValue*24/4096) % 24) * 100
#define ADC_READ_MINUTE		((BaseTime%100 + ADCValue*60/4096) % 60)

#include <stdint.h>
#include "ST7735.h"
#include "PLL.h"
#include "SysTickDriver.h"
#include "SpeakerDriver.h"
#include "ST7735.h"
#include "SwitchDriver.h"
#include "ADCSWTrigger.h"
#include "../inc/tm4c123gh6pm.h"


const uint16_t SIN_FIXED[] = {0, 10, 20, 30, 40, 49, 58, 66, 74, 80,
														86, 91, 95, 97, 99, 100, 99, 97, 95, 91,
														86, 80, 74, 66, 58, 49, 40, 30, 20, 10,
														0, -10, -20, -30, -40, -49, -58, -66, -74, -80,
														-86, -91, -95, -97, -99, -100, -99, -97, -95, -91,
														-86, -80, -74, -66, -58, -49, -40, -30, -20, -10};

const uint16_t COS_FIXED[] = {100, 99, 97, 95, 91, 86, 80, 74, 66, 58,
															50, 40, 30, 20, 10, 0, -10, -20, -30, -40,
															-49, -58, -66, -74, -80, -86, -91, -95, -97, -99,
															-100, -99, -97, -95, -91, -86, -80, -74, -66, -58,
															-50, -40, -30, -20, -10, 0, 10, 20, 30, 40,
															50, 58, 66, 74, 80, 86, 91, 95, 97, 99};
														
// state values
const uint32_t STANDARD_CLOCK = 0;
const uint32_t MILITARY_CLOCK = 1;
const uint32_t SET_TIME_HOUR = 2;
const uint32_t SET_TIME_MINUTE = 3;
const uint32_t SET_ALARM_HOUR = 4;
const uint32_t SET_ALARM_MINUTE = 5;

static uint32_t State;		// current state
static uint32_t AlarmOn = 0;							// 1 if alarm is on
static uint32_t AlarmTime = 0;						// time of alarm
static volatile uint32_t ADCValue = 0;		// adc value
static uint32_t HomeState = 0;
static uint32_t BaseTime = 0;
static uint32_t NewTime = 0;
static uint32_t OldTime = 0;
extern uint32_t AtomicTime;								// current time

// start set time state, or move to next state
void Button1SetTime(void) { 
	if(State == SET_TIME_HOUR) {
		NewTime = ADC_READ_HOUR;
		State = SET_TIME_MINUTE;
	} else if (State == SET_TIME_MINUTE) {
		NewTime += ADC_READ_MINUTE;
		AtomicTime = NewTime;
		State = HomeState;
	} else {
		BaseTime = AtomicTime;
		State = SET_TIME_HOUR;
	}
}

// start set alarm state, or move to next state
void Button2SetAlarm(void) {
	if(State == SET_ALARM_HOUR) {
		NewTime = ADC_READ_HOUR;
		State = SET_ALARM_MINUTE;
	} else if (State == SET_ALARM_MINUTE) {
		NewTime += ADC_READ_MINUTE;
		AlarmTime = NewTime;
		State = HomeState;
	} else {
		BaseTime = AlarmTime;
		State = SET_ALARM_HOUR;
	}
}

// enable/disable alarm
void Buttton3ToggleAlarm(void) {
	AlarmOn = 1 - AlarmOn;
}

// move to next state
void Button4ChangeMode(void) {
	if(State == STANDARD_CLOCK) {
		State = MILITARY_CLOCK;
	} else if(State == MILITARY_CLOCK) {
		State = STANDARD_CLOCK;
	} else {
		State = 1 - HomeState;
	}
}

const uint32_t X_CLOCK_CENTER;
const uint32_t Y_CLOCK_CENTER;
const uint32_t HOUR_HAND_LENGTH;
const uint32_t MINUTE_HAND_LENGTH;

void DrawHands(uint32_t time, uint16_t color) {
	uint32_t angleHour = (((time/100) % 12)*60 + time%100)/12;
	uint32_t xValueHour = X_CLOCK_CENTER - SIN_FIXED[angleHour]/5;
	uint32_t yValueHour = Y_CLOCK_CENTER + COS_FIXED[angleHour]/5;
	ST7735_Line(X_CLOCK_CENTER, Y_CLOCK_CENTER, xValueHour, yValueHour, color);
	
	uint32_t angleMinute = time%100;
	uint32_t xValueMinute = X_CLOCK_CENTER - SIN_FIXED[angleMinute]/4;
	uint32_t yValueMinute = Y_CLOCK_CENTER + COS_FIXED[angleMinute]/4;
	ST7735_Line(X_CLOCK_CENTER, Y_CLOCK_CENTER, xValueMinute, yValueMinute, color);
}

// draw clock and lines
void DrawClock(char* title, uint32_t time) {
	// clear old values 
	DrawHands(OldTime, ST7735_WHITE);
	OldTime = time;
	// draw title
	ST7735_SetCursor(0,0);
	ST7735_OutString(title);
	ST7735_OutString("\rAlarm ");
	if(AlarmOn) {
		ST7735_OutString("On");
	} else {
		
	ST7735_OutString("Off");
	}
	// draw analog clock face
	DrawHands(time, ST7735_BLACK);
	// draw digital clock
	ST7735_OutUDec(time/100);
	ST7735_OutString(":");
	ST7735_OutUDec(time%100);
	// draw standard vs military
	if(HomeState == STANDARD_CLOCK) {
		ST7735_OutUDec((time/100)%12);
		ST7735_OutString(":");
		ST7735_OutUDec(time%100);
		if(time/100 < 12){
			ST7735_OutString(" AM");
		}
		else{
			ST7735_OutString(" PM");
		}
	} else {
		ST7735_OutUDec(time/100);
		ST7735_OutString(":");
		ST7735_OutUDec(time%100);
	}
	
}

void DisplayClock(void) {
	uint32_t time = AtomicTime;
	char* screen;
	if(State == STANDARD_CLOCK) {					// draw clock with time from systick
		screen = "Clock";
	} else if(State == MILITARY_CLOCK) {	// draw clock with time from systick
		screen = "Clock";
	} else if(State == SET_TIME_HOUR) {		// draw clock with time from start time 
																				// from systick offset by adc
		uint32_t time2 = ADC_READ_HOUR;
		time = time2 + time % 100;
		screen = "Set Time - Hours";
	} else if(State == SET_TIME_MINUTE) {	// draw clock with time from start time 
																				// from systick offset by adc
		uint32_t time2 = ADC_READ_MINUTE;
		time = (time/100)*100 + time2;
		screen = "Set Time - Minutes";
	} else if(State == SET_ALARM_HOUR) {	// draw clock with alarm time offset by adc
		uint32_t time2 = ADC_READ_HOUR;
		time = time2 + AlarmTime % 100;
		screen = "Set Alarm - Hours";
	} else if(State == SET_ALARM_MINUTE) {	// draw clock with alarm time offset by adc
		uint32_t time2 = ADC_READ_MINUTE;
		time = (AlarmTime/100)*100 + time2;
		screen = "Set Alarm - Minutes";
	}
	DrawClock(screen,time);
}

int main(void) {
	PLL_Init(Bus80MHz);				// init modules
	SysTick_Init();
	Speaker_Init();
	ST7735_InitR(INITR_REDTAB);
	Switch_Init(Button1SetTime, Button2SetAlarm, Buttton3ToggleAlarm,
			Button4ChangeMode);
	ADC0_InitSWTriggerSeq3_Ch9();
	HomeState = STANDARD_CLOCK;
	State = HomeState;
	
	while(1) {
		DisplayClock();
	}
}


