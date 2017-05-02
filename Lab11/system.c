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
#include "FSM.h"
#include "esp8266.h"
#include "../inc/tm4c123gh6pm.h"

#include "switch.h"
#include "keypad.h"
#include "speaker.h"
#include "solenoid.h"
#include "ST7735.h"
#include "esp8266.h"

int main1(void) {
	PLL_Init(Bus80MHz);				// init modules
	System_Init();
	while(1) {
		ChangeState();
		UpdateState();
	}
}

