// main.c
// Runs on LM4F120/TM4C123
// UART runs at 115,200 baud rate 
// Daniel Valvano
// May 3, 2015

/* This example accompanies the books
  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

"Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
 
 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
 
 /*
Three ways to initiate ADC conversion process
		Software Trigger
		Timer
		Continuous sample
Completed when
		ADC sampler interrupt
		ADC RIS register will have flags set
*/



#include <stdint.h> // C99 variable types
#include "..//inc//tm4c123gh6pm.h"
#include "ADCSWTrigger.h"
#include "uart.h"
#include "PLL.h"

uint16_t const ADCdata[53]={0,104,134,165,196,228,261,294,328,362,397,
     433,470,507,545,584,624,665,706,748,791,
     834,879,925,971,1018,1066,1115,1165,1216,1268,
     1321,1375,1430,1486,1543,1601,1660,1721,1782,1844,
     1908,1973,2038,2105,2173,2243,2313,2385,2457,2531,2607,4096};


uint16_t const Tdata[53]={4000,4000,3940,3880,3820,3760,3700,3640,3580,3520,3460,
     3400,3340,3280,3220,3160,3100,3040,2980,2920,2860,
     2800,2740,2680,2620,2560,2500,2440,2380,2320,2260,
     2200,2140,2080,2020,1960,1900,1840,1780,1720,1660,
     1600,1540,1480,1420,1360,1300,1240,1180,1120,1060,1000,1000};

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode


// ***************** Timer0A_Init ****************
// Activate TIMER0 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq), 32 bits
// Outputs: none
void Timer0A_Init(uint32_t period){long sr;
  sr = StartCritical(); 
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = period-1;    // 4) reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
  EndCritical(sr);
}

volatile uint32_t Values[100] = {0};
volatile uint32_t Count = 0;

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
  if(Count < 100) Values[Count] = ADC0_InSeq3();
}

uint32_t convert(uint32_t adcValue) {
	int j = 0;
	while(j < 100-1 && adcValue >= ADCdata[j+1]) {
		j++;
	}
	uint32_t temp = (adcValue - ADCdata[j])*(Tdata[j+1]-Tdata[j])/(ADCdata[j+1]-ADCdata[j]) + Tdata[j];
	return temp;
}

int main(void){ int32_t data;
  PLL_Init(Bus80MHz);   // 80 MHz
  UART_Init();              // initialize UART device
  Timer0A_Init(80000);  // initialize timer0A (1000 Hz)
  ADC0_InitSWTriggerSeq3_Ch9();
  while(Count < 100){
		WaitForInterrupt();
    UART_OutString("\n\rADC data =");
    UART_OutUDec(data);
  }
	for(int i = 0; i < 100; i++) {
    UART_OutString("\n\rADC data[");
    UART_OutUDec(i);
		UART_OutString("] =");
    UART_OutUDec(convert(Values[i]));
	}
}


