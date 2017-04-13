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
#include "ST7735.h"

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
//	int j = 0;
//	while(j < 100-1 && adcValue >= ADCdata[j+1]) {
//		j++;
//	}
//	uint32_t temp = (adcValue - ADCdata[j])*((Tdata[j+1]-Tdata[j])/(ADCdata[j+1]-ADCdata[j])) + Tdata[j];
//	return temp;
	uint32_t temp = 0;
	for(int i = 0; i < 53; i++){
		if (adcValue == ADCdata[i]) {
						temp = Tdata[i];
						break;
					} else if (adcValue > ADCdata[i]) {
						continue;
					} else {
						uint32_t upperX = ADCdata[i];
						uint32_t lowerX = ADCdata[i-1];
						uint32_t upperY = Tdata[i];
						uint32_t lowerY = Tdata[i-1];
						int32_t xDiff = upperX - lowerX;
						int32_t yDiff = upperY - lowerY;
						int32_t slope = (yDiff) / (xDiff);
						temp = ((adcValue - lowerX) * slope) + lowerY;
						break;
					}
	}
	return temp;
}

const int32_t MAX_DECIMAL = 4001;
const int32_t MIN_DECIMAL = 999;

/****************ST7735_sDecOut3***************
 converts fixed point number to LCD
 format signed 32-bit with resolution 0.001
 range -9.999 to +9.999
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
 */
void ST7735_sDecOut2(int32_t n) {
	if(n < MAX_DECIMAL && n > MIN_DECIMAL) { // if valid n
		char output[] = "   .  ";
		// if negative, add sign and change to positive
		if(n < 0) { 
			output[0] = '-';
			n *= -1;
		}
		// create output
		for(uint32_t i = 5; i > 1; i--) {
			if(i==3) i--;
			uint32_t nextDigit = n % 10;
			n = n / 10;
			output[i] = ('0' + nextDigit);	
		}
		output[1] = ('0' + n);
		// display
		ST7735_OutString(output);
	} else { 
		// print error if not valid n
		ST7735_OutString(" **.**");
	}
}

int main(void){
	ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_SetCursor(0,0); ST7735_OutString("Lab 9: Thermistor");
  ST7735_PlotClear(1000,4000);  // range from 0 to 4095
  ST7735_SetCursor(0,1); ST7735_OutString("N=");
  ST7735_SetCursor(0,2); ST7735_OutString("T=");
                        
  PLL_Init(Bus80MHz);   // 80 MHz
  UART_Init();              // initialize UART device
  //Timer0A_Init(80000);  // initialize timer0A (1000 Hz)
	Timer0A_Init(1600000); //50hz
  ADC0_InitSWTriggerSeq3_Ch9();
	
  while(Count < 100){
		WaitForInterrupt();
    //UART_OutString("\n\rADC data =");
    //UART_OutUDec(data);
		uint32_t temperature = convert(Values[Count]);
		ST7735_PlotPoint(temperature);  // Measured temperature
		ST7735_PlotNext();
    ST7735_SetCursor(3,1); ST7735_OutUDec(Values[Count]);            // 0 to 4095
    ST7735_SetCursor(2,2); ST7735_sDecOut2(temperature); ST7735_OutString(" C");// 0.01 C 
		Count++;
  }
	for(int i = 0; i < 100; i++) {
    UART_OutString("\n\rADC data[");
    UART_OutUDec(i);
		UART_OutString("] = ");
    //UART_OutUDec(convert(Values[i]));
		UART_OutUDec(Values[i]);

	}
	

}


