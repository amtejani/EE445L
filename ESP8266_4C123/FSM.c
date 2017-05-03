// ******** FSM.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 4/19/2017
// system interface
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 4/19/2017

#include <stdint.h>
#include "PLL.h"
#include "switch.h"
#include "keypad.h"
#include "speaker.h"
#include "solenoid.h"
#include "ST7735.h"
#include "esp8266.h"
#include "UART.h"
#include "../inc/tm4c123gh6pm.h"

#define OFF 0
#define ON 1
#define OPEN 2
#define CHANGE_PASS 3
#define NEW_PASS 4
#define CONFIRM_PASS 5
#define WAITING 6

#define KEYCODE_LENGTH 4

#define KEYCODE_TIMEOUT 80000000*10
#define WAITING_TIMEOUT 80000000*10

char CHANGE_CODE[] = "GET /changecode?oldcode=0000&newcode1=0000&newcode2=0000 HTTP/1.1\r\nHost:securityserver-165920.appspot.com\r\n\r\n";
char CHECK_CODE[] = "GET /checkcode?code=0000 HTTP/1.1\r\nHost:securityserver-165920.appspot.com\r\n\r\n";
char SEND_STATUS[] = "GET /status?state=0&door=0 HTTP/1.1\r\nHost:securityserver-165920.appspot.com\r\n\r\n";
char LOCK_RESPONSE[] = "lock=";
char NEW_CODE_RESPONSE[] = "succ=";
char TEST_CODE_RESPONSE[] = "good=";
//#define SEND_STATUS "POST /status HTTP/1.1\r\nHost:securityserver-165920.appspot.com\r\nUser-Agent: Keil\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 14\r\n\r\nstate=0&door=0\r\n\r\n"
//#define SEND_STATUS "GET /status?state=0&door=0 HTTP/1.1\r\nHost:securityserver-165920.appspot.com\r\n\r\n"
//char SEND_STATUS[] = "GET /data/2.5/weather?q=Austin%20Texas&APPID=1842c4db572feb98a55e1d096f86c57b HTTP/1.1\r\nHost:api.openweathermap.org\r\n\r\n";

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

uint32_t KeypadCounter;
char KeycodeInput[KEYCODE_LENGTH];

char TempKeycode0[KEYCODE_LENGTH];
char TempKeycode1[KEYCODE_LENGTH];
char TempKeycode2[KEYCODE_LENGTH];

uint32_t State;
uint32_t DoorStatus;
uint32_t ChangeCode;
uint32_t Enable;

uint32_t KeycodeTime;
uint32_t TimeoutTime;

#define TIMEOUT_ERROR "ur too slow scrub"
#define INVALID_INPUT "Incorrect keycode"
#define NO_ERROR  "                 "
#define SUCCESS  "Success!         "


// On Enable button press, turn on system
static void EnableSystem() {
	if(State == OFF) {
		Enable = 1;
	}
}

static void ResetState(){
	ChangeCode = 0;
	Enable = 0;
	TimeoutTime = TIMER3_TAR_R;
	KeypadCounter = 0;
}

// On Change button press, allow ask for passcode and allow user to 
// change passcode
static void ChangeKeycode() {
	if(State == ON || State == WAITING || State == OFF) {
		ChangeCode = 1;
	}
}

// On door open, start alarm
static void MagnetOpen() {
	if(State == ON) {
		DoorStatus = 1;
	}
}

// On keypad press, record button pressed
static void KeypadPress(uint32_t pressed) {
	if(KeypadCounter < KEYCODE_LENGTH) {
		KeycodeInput[KeypadCounter++] = '0' + pressed;
	}
}


static char* SearchStr(char* string, char* searchingString) {
	char* ptr = searchingString;
	while(*string != 0) {
		if(*searchingString == 0) return string;
		if(*searchingString == *string) {
			searchingString++;
		} else {
			searchingString = ptr;
		}
		string++;
	}
	return 0;
}


// validate key from server
static uint32_t ValidateKey() {
	int ret = 1;
	if(ESP8266_MakeTCPConnection("securityserver-165920.appspot.com")){ // open socket in server
		int i;
		for(i = 0; i < KEYCODE_LENGTH; i++) {
			CHECK_CODE[20 + i] = KeycodeInput[i];
		}
    ESP8266_SendTCP(CHECK_CODE);
		//if (ServerResponseBuffer[231] == '1') // return 0 if key is valid 
		//	ret = 0;
		char* buff = SearchStr(ServerResponseBuffer,TEST_CODE_RESPONSE);
		if(buff) {
			if (*buff == '1')
				ret = 0;
		}
		
  }
  ESP8266_CloseTCPConnection();
	return ret;
}
// update key in server
static uint32_t UpdateKey() {
	int ret = 1;
	if(ESP8266_MakeTCPConnection("securityserver-165920.appspot.com")){ // open socket in server
    ESP8266_SendTCP(CHANGE_CODE);
		char* buff = SearchStr(ServerResponseBuffer,NEW_CODE_RESPONSE);
		if(buff) {
			if (*buff == '1')
				ret = 0;
		}
  }
  ESP8266_CloseTCPConnection();
	return ret;
}

uint32_t UpdateState() {
	ESP8266_GetStatus();
	if(ESP8266_MakeTCPConnection("securityserver-165920.appspot.com")){ // open socket in server
		SEND_STATUS[18] = State + '0';
		SEND_STATUS[25] = DoorStatus + '0';
    ESP8266_SendTCP(SEND_STATUS);
		char* buff = SearchStr(ServerResponseBuffer,LOCK_RESPONSE);
		if(buff) {
			if (*buff == '1')
				Solenoid_Out(0);
			else 
				Solenoid_Out(1);
		}
  }
  ESP8266_CloseTCPConnection();
	return 0;
}

static uint32_t StateOff() {
	if(Enable) {
		State = WAITING;
	} else if(ChangeCode) {
		State = CHANGE_PASS;
	}
	ResetState();
	KeycodeTime = TIMER4_TAR_R;
	return 0;
}

static uint32_t StateWaiting() {
	if(KeycodeTime - TIMER3_TAR_R < WAITING_TIMEOUT) {
		if(KeypadCounter == 0) {
			ResetState();
		} else if(KeypadCounter > 0 && TimeoutTime - TIMER4_TAR_R > KEYCODE_TIMEOUT) {
			ResetState();
			// display timeout message
			ST7735_SetCursor(0,1);
			ST7735_OutString(TIMEOUT_ERROR);
		} else if(KeypadCounter == KEYCODE_LENGTH) {
			int result = ValidateKey();
			if(!result) {
				State = OFF;
				ResetState();
				KeycodeTime = TIMER4_TAR_R;
				ST7735_SetCursor(0,1);
				ST7735_OutString(NO_ERROR);
				return 0;
			} else {
				// display error message
				ST7735_SetCursor(0,1);
				ST7735_OutString(INVALID_INPUT);
				ResetState();
			}
		}
	} else {
		State = ON;
		ResetState();
		KeycodeTime = TIMER4_TAR_R;
		ST7735_SetCursor(0,1);
		ST7735_OutString(NO_ERROR);
	}
	return 0;
}

static uint32_t StateOn() {
	if(KeypadCounter == 0) {
		ResetState();
	} else if(KeypadCounter > 0 && TimeoutTime - TIMER4_TAR_R > KEYCODE_TIMEOUT) {
		ResetState();
		// display timeout message
		ST7735_SetCursor(0,1);
		ST7735_OutString(TIMEOUT_ERROR);
	} else if(KeypadCounter == KEYCODE_LENGTH) {
		int result = ValidateKey();
		ResetState();
		if(!result) {
			State = OFF;
			ST7735_SetCursor(0,1);
			ST7735_OutString(NO_ERROR);
			return 0;
		} else {
			// display error message
			ST7735_SetCursor(0,1);
			ST7735_OutString(INVALID_INPUT);
		}
	} 
	if(GetDoorStatus() == 1) { 
		State = OPEN;
		ST7735_SetCursor(0,1);
		ST7735_OutString(NO_ERROR);
		ResetState();
	}
	return 0;
}

static uint32_t StateOpen() {
	SpeakerEnable();
	if(KeypadCounter == 0) {
		ResetState();
	} else if(KeypadCounter > 0 && TimeoutTime - TIMER4_TAR_R > KEYCODE_TIMEOUT) {
		ResetState();
		// display timeout message
		ST7735_SetCursor(0,1);
		ST7735_OutString(TIMEOUT_ERROR);
	} else if(KeypadCounter == KEYCODE_LENGTH) {
		int result = ValidateKey();
		ResetState();
		if(!result) {
			State = OFF;
			ST7735_SetCursor(0,1);
			ST7735_OutString(NO_ERROR);
			SpeakerDisable();
			return 0;
		} else {
			// display error message
			ST7735_SetCursor(0,1);
			ST7735_OutString(INVALID_INPUT);			
		}
	}
	return 0;
}

static uint32_t StateChange1() {
	if(TimeoutTime - TIMER4_TAR_R < KEYCODE_TIMEOUT) {
		if(KeypadCounter == KEYCODE_LENGTH) {
			int result = ValidateKey();
			int i;
			for(i = 0; i < KEYCODE_LENGTH; i++) {
				CHANGE_CODE[24 + i] = KeycodeInput[i];
			}
			ResetState();
			if(!result) {
				State = NEW_PASS;
				ST7735_SetCursor(0,1);
				ST7735_OutString(NO_ERROR);
				return 0;
			} else {
				// display error message
				ST7735_SetCursor(0,1);
				ST7735_OutString(INVALID_INPUT);				
				State = OFF;
				return 0;
			}
		}
	} else {
		// display timeout message
		ST7735_SetCursor(0,1);
		ST7735_OutString(TIMEOUT_ERROR);
		State = OFF;
		ST7735_SetCursor(0,1);
		ST7735_OutString(NO_ERROR);
		ResetState();
	}
	return 0;
}

static uint32_t StateChange2() {
	if(TimeoutTime - TIMER4_TAR_R < KEYCODE_TIMEOUT) {
		if(KeypadCounter == KEYCODE_LENGTH) {
			int i;
			for(i = 0; i < KEYCODE_LENGTH; i++) {
				CHANGE_CODE[38 + i] = KeycodeInput[i];
			}
			State = CONFIRM_PASS;
			ResetState();
		}
	} else {
		State = OFF;
		ST7735_SetCursor(0,1);
		ST7735_OutString(NO_ERROR);
		ResetState();
	}
	return 0;
}

static uint32_t StateChange3() {
	if(TimeoutTime - TIMER4_TAR_R < KEYCODE_TIMEOUT) {
		if(KeypadCounter == KEYCODE_LENGTH) {
			int i;
			for(i = 0; i < KEYCODE_LENGTH; i++) {
				CHANGE_CODE[52 + i] = KeycodeInput[i];
			}
			if(!UpdateKey()) {
				// Display success message
				ST7735_SetCursor(0,1);
				ST7735_OutString(SUCCESS);
			} else {
				// Display incorrect input message
				ST7735_SetCursor(0,1);
				ST7735_OutString(INVALID_INPUT);
			}
			State = OFF;
			ResetState();
		}
	} else {
		State = OFF;
		ResetState();
	}
	return 0;
}

// Execute state functions and change states if necessary
void ChangeState() {
	ST7735_SetCursor(0,0);
	if(State == OFF) {
		ST7735_OutString("Alarm Off          ");
		StateOff();
	} else if (State == WAITING) {
		ST7735_OutString("Alarm on in 5 sec  ");		
		StateWaiting();
	} else if (State == ON) {
		ST7735_OutString("Alarm Enabled      ");		
		StateOn();
	} else if(State == OPEN) {
		ST7735_OutString("Door breached..run!");		
		StateOpen();
	} else if (State == CHANGE_PASS) {
		ST7735_OutString("Input old keycode  ");		
		StateChange1();
	} else if (State == NEW_PASS) {
		ST7735_OutString("Input new keycode  ");		
		StateChange2();
	} else if (State == CONFIRM_PASS) {
		ST7735_OutString("Confirm new keycode");
		StateChange3();
	}
}

static void Timer3_Init() {
	SYSCTL_RCGCTIMER_R |= 0x08;      // activate timer3
  int delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER3_CTL_R = 0x00000000;    // 1) disable TIMER3 during setup
  TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER3_TAMR_R = TIMER_TAMR_TAMR_PERIOD;  // 3) 1-SHOT mode
  TIMER3_TAILR_R = 0xFFFFFFFF;  // 4) reload value
  TIMER3_TAPR_R = 0;            // 5) bus clock resolution
  TIMER3_CTL_R = 0x00000001;    // 10) enable TIMER3
}
static void Timer4_Init() {
	SYSCTL_RCGCTIMER_R |= 0x10;      // activate timer4
  int delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER4_CTL_R = 0x00000000;    // 1) disable TIMER3 during setup
  TIMER4_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER4_TAMR_R = TIMER_TAMR_TAMR_PERIOD;    // 3) 1-SHOT mode
  TIMER4_TAILR_R = 0xFFFFFFFF;  // 4) reload value
  TIMER4_TAPR_R = 0;            // 5) bus clock resolution
  TIMER4_CTL_R = 0x00000001;    // 10) enable TIMER3
}

void System_Init(void) {
  DisableInterrupts();
	Timer3_Init();
	Timer4_Init();
	ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(ST7735_BLACK);
	Switch_Init(EnableSystem,ChangeKeycode,MagnetOpen);
	Keypad_Init(KeypadPress);
	Solenoid_Init();
	Speaker_Init();
	State = OFF;
	Output_Init();
  ESP8266_Init(115200);      // connect to access point, set up as client
  ESP8266_GetVersionNumber();
}


