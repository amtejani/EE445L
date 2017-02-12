// ******** ClockMain.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/8/2017
// Main to test modules for clock
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/9/2017


#include <stdint.h>
#include "ST7735.h"
#include "PLL.h"
#include "SysTickDriver.h"
#include "SpeakerDriver.h"
#include "ST7735.h"
#include "SwitchDriver.h"
#include "ADCSWTrigger.h"
#include "../inc/tm4c123gh6pm.h"

// state values
const uint32_t DISPLAY_CLOCK = 0;
const uint32_t SET_TIME_HOUR = 1;
const uint32_t SET_TIME_MINUTE = 2;
const uint32_t SET_ALARM_HOUR = 3;
const uint32_t SET_ALARM_MINUTE = 4;

static uint32_t State = DISPLAY_CLOCK;		// current state
static uint32_t AlarmOn = 0;							// 1 if alarm is on
static uint32_t AlarmTime = 0;						// time of alarm
static volatile uint32_t ADCValue = 0;		// adc value
extern uint32_t AtomicTime;								// current time

// start set time state, or move to next state
void button1SetTime(void) { 
	if(State != SET_TIME_HOUR && State != SET_TIME_MINUTE) {
		State = SET_TIME_HOUR;
	} else {
		State = DISPLAY_CLOCK;
	}
}

// start set alarm state, or move to next state
void button2SetAlarm(void) {
	if(State != SET_ALARM_HOUR && State != SET_ALARM_MINUTE) {
		State = SET_ALARM_HOUR;
	} else {
		State = DISPLAY_CLOCK;
	}
}

// enable/disable alarm
void buttton3ToggleAlarm(void) {
	AlarmOn = 1 - AlarmOn;
}

// move to next state
void button4ChangeMode(void) {
	if(State == SET_TIME_HOUR) {
		State = SET_TIME_MINUTE;
	} else if(State == SET_TIME_MINUTE) {
		State = DISPLAY_CLOCK;
		// set AtomicTime
	} else if(State == SET_ALARM_HOUR) {
		State = SET_ALARM_MINUTE;
	} else if(State == SET_ALARM_MINUTE) {
		State = DISPLAY_CLOCK;
		// set AlarmTime
	}
}

// draw clock and lines
void drawClock(char* title, uint32_t time) {
	// clear old values 
	
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
	// draw digital clock
	ST7735_OutUDec(time/100);
	ST7735_OutString(":");
	ST7735_OutUDec(time%100);
	
}

int main(void) {
	PLL_Init(Bus80MHz);				// init modules
	SysTick_Init();
	Speaker_Init();
	ST7735_InitR(INITR_REDTAB);
	Switch_Init(button1SetTime, button2SetAlarm, buttton3ToggleAlarm,
			button4ChangeMode);
	ADC0_InitSWTriggerSeq3_Ch9();
	
	while(1) {
		uint32_t time = AtomicTime;
		if(State == DISPLAY_CLOCK) {					// draw clock with time from systick
			drawClock("Clock", time);
		} else if(State == SET_TIME_HOUR) {		// draw clock with time from start time 
																					// from systick offset by adc
			// sample adc
			uint32_t hour = ADCValue / 24;
			time = hour *100 + time%100;
			drawClock("Set Time - Hours", time);
		} else if(State == SET_TIME_MINUTE) {	// draw clock with time from start time 
																					// from systick offset by adc
			// sample adc
			uint32_t minute = ADCValue / 60;
			time = time /100 + minute;
			drawClock("Set Time - Minutes",time);
		} else if(State == SET_ALARM_HOUR) {	// draw clock with alarm time offset by adc
			uint32_t hour = ADCValue / 24;
			time = hour /100 + AlarmTime%100;
			drawClock("Set Alarm - Hours",time);
		} else if(State == SET_ALARM_MINUTE) {	// draw clock with alarm time offset by adc
			uint32_t minute = ADCValue / 60;
			time = AlarmTime /100 + minute;
			drawClock("Set Alarm - Minutes",time);
		}
	}
}


