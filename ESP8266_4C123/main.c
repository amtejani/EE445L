//***********************  main.c  ***********************
// Program written by:
// - Steven Prickett  steven.prickett@gmail.com
//
// Brief desicription of program:
// - Initializes an ESP8266 module to act as a WiFi client
//   and fetch weather data from openweathermap.org
//
//*********************************************************
/* Modified by Jonathan Valvano
 Sept 14, 2016
 Out of the box: to make this work you must
 Step 1) Set parameters of your AP in lines 59-60 of esp8266.c
 Step 2) Change line 39 with directions in lines 40-42
 Step 3) Run a terminal emulator like Putty or TExasDisplay at
         115200 bits/sec, 8 bit, 1 stop, no flow control
 Step 4) Set line 50 to match baud rate of your ESP8266 (9600 or 115200)
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/tm4c123gh6pm.h"

#include "pll.h"
#include "UART.h"
#include "esp8266.h"
#include "FSM.h"
#include "solenoid.h"

// prototypes for functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

//char Fetch[] = "GET /data/2.5/weather?q=Austin%20Texas&APPID=1842c4db572feb98a55e1d096f86c57b HTTP/1.1\r\nHost:api.openweathermap.org\r\n\r\n";
char Fetch[] = "GET /status?state=0&door=0 HTTP/1.1\r\nHost:securityserver-165920.appspot.com\r\n\r\n";
// 1) go to http://openweathermap.org/appid#use 
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 1234567890abcdef1234567890abcdef with your APPID
void DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}
int main(void){  
  DisableInterrupts();
  PLL_Init(Bus80MHz);
//  Output_Init();       // UART0 only used for debugging
//  printf("\n\r-----------\n\rSystem starting...\n\r");
//  ESP8266_Init(115200);      // connect to access point, set up as client
//  ESP8266_GetVersionNumber();
	System_Init();
	int i = 0;
  while(1){
		ChangeState();
		i++;
		if(i%1000== 0) {
			UpdateState();
		}
  }
}

// transparent mode for testing
void ESP8266SendCommand(char *);
int main2(void){  char data;
  DisableInterrupts();
  PLL_Init(Bus80MHz);
  Output_Init();       // UART0 as a terminal
  printf("\n\r-----------\n\rSystem starting at 115200 baud...\n\r");
//  ESP8266_Init(38400);
  ESP8266_InitUART(115200,true);
  ESP8266_EnableRXInterrupt();
  EnableInterrupts();
  ESP8266SendCommand("AT+RST\r\n");
  data = UART_InChar();
//  ESP8266SendCommand("AT+UART=115200,8,1,0,3\r\n");
//  data = UART_InChar();
//  ESP8266_InitUART(115200,true);
//  data = UART_InChar();
  
  while(1){
// echo data back and forth
    data = UART_InCharNonBlock();
    if(data){
      ESP8266_PrintChar(data);
    }
  }
}



