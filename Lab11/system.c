// ******** system.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 4/19/2017
// system interface
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 4/19/2017

#include <stdint.h>
#include <string.h>
#include "PLL.h"
#include "switch.h"
#include "keypad.h"
#include "speaker.h"
#include "solenoid.h"
#include "ST7735.h"

#define OFF 0
#define ON 1
#define OPEN 2
#define CHANGE_PASS 3
#define NEW_PASS 4
#define CONFIRM_PASS 5

#define KEYCODE_LENGTH 4


uint32_t KeypadCounter;
char* KeycodeInput;

char* TempKeycode1;
char* TempKeycode2;

uint32_t State;
uint32_t DoorStatus;
uint32_t ChangeCode;
uint32_t Enable;

// On Enable button press, turn on system
void EnableSystem() {
	if(State == OFF) {
		Enable = 1;
	}
}

// On Change button press, allow ask for passcode and allow user to 
// change passcode
void ChangeKeycode() {
	if(State == ON || State == OFF) {
		ChangeCode = 1;
	}
}

// On door open, start alarm
void MagnetOpen() {
	if(State == ON) {
		State = OPEN;
	}
}

// On keypad press, record button pressed
void KeypadPress(uint32_t pressed) {
	if(KeypadCounter < KEYCODE_LENGTH) {
		KeycodeInput[KeypadCounter++] = pressed;
	}
}


// validate key from server
uint32_t ValidateKey(char* key) {
	return 1;
}
// update key in server
uint32_t UpdateKey(char* key) {
	return 1;
}

// wait for a valid keycode, and validate
uint32_t CheckKeycodeAlarmOn() {
	KeypadCounter = 0;
	// get time
	while(KeypadCounter < KEYCODE_LENGTH) {
		while(KeypadCounter == 0);
		// while (timer < 5 secs);
		// if (KeypadCounter > 0) 
			// wait for 5 seconds before resetting counter
			// get new time
	}
	KeypadCounter = 0;
	return ValidateKey(KeycodeInput);
}

// wait for valid keycode or change state 
uint32_t CheckKeycodeAlarmEnabled() {
	KeypadCounter = 0;
	// get time
	while(KeypadCounter < KEYCODE_LENGTH) {
		while(KeypadCounter == 0 && !ChangeCode);
		// while (timer < 5 secs);
		if(ChangeCode) return 0;
		// if (KeypadCounter > 0) 
			// wait for 5 seconds before resetting counter
			// get new time
	}
	KeypadCounter = 0;
	return ValidateKey(KeycodeInput);
}

// check if keycode valid, timeout in 5 seconds
uint32_t CheckKeycodeOnce() {
	KeypadCounter = 0;
	
	while(KeypadCounter < KEYCODE_LENGTH) {
		//get time
		// if (timer > 5 seconds) 
			// wait for 5 seconds before quitting
			// KeypadCounter = 0;
			// return 0;
	}
	KeypadCounter = 0;
	return ValidateKey(KeycodeInput);
}

// read keycode to change passcode
uint32_t ReadNewKeycode(char* in) {
	KeypadCounter = 0;
	// get time
	while(KeypadCounter < KEYCODE_LENGTH) {
		// if (timer > 5 seconds) 
			// wait for 5 seconds before quitting
			// KeypadCounter = 0;
			// return 0
	}
	KeypadCounter = 0;
	strcpy(in, KeycodeInput);
	return 1;
}

// Execute state functions and change states if necessary
void ChangeState() {
	if(State == OFF) {
		if(Enable) State = ON;
		else if(ChangeCode) State = CHANGE_PASS;
		Enable = 0;
		ChangeCode = 0;
	} else if (State == ON) {
		if(CheckKeycodeAlarmEnabled()) {
			State = OFF;
		} else {
			if(ChangeCode) {
				ChangeCode = 0;
				State = CHANGE_PASS;
			} else {
				// Display incorrect input message
			}
		}
	} else if(State == OPEN) {
		SpeakerEnable();
		if(CheckKeycodeAlarmOn()) {
			SpeakerDisable();
			State = OFF;
		} else {
			// Display incorrect input message
		}
	} else if (State == CHANGE_PASS) {
		if(CheckKeycodeOnce()) {
			State = NEW_PASS;
		} else {
			// Display incorrect input message
		}
	} else if (State == NEW_PASS) {
		if(ReadNewKeycode(TempKeycode1)) {
			State = CONFIRM_PASS;
		} else {
			// Display timeout message
		}
	} else if (State == CONFIRM_PASS) {
		if(ReadNewKeycode(TempKeycode2)) {
			if(strcmp(TempKeycode1,TempKeycode2)) {
				UpdateKey(TempKeycode1);
			} else {
				// Display incorrect input message
			}
			State = OFF;
		} else {
			// Display timeout message
		}
	}
}

int main(void) {
	PLL_Init(Bus80MHz);				// init modules
	ST7735_InitR(INITR_REDTAB);
	Switch_Init(EnableSystem,ChangeKeycode,MagnetOpen);
	Keypad_Init(KeypadPress);
	Speaker_Init();
	while(1) {
		ChangeState();
	}
}

