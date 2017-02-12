// ******** ADCTestMain.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 1/31/2017
// Possible main program to test the lab 2
// Runs on TM4C123
// Uses ST7735.c LCD.
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/7/2017


/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
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

// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V 
#include <stdint.h> 
#include "ST7735.h"
#include "ADCSWTrigger.h"
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"

#define PF2             (*((volatile uint32_t *)0x40025010))
#define PF1             (*((volatile uint32_t *)0x40025008))
void DisableInterrupts(void); 		// Disable interrupts
void EnableInterrupts(void);  		// Enable interrupts
long StartCritical (void);    		// previous I bit, disable interrupts
void EndCritical(long sr);    		// restore I bit to previous value
void WaitForInterrupt(void);  		// low power mode
void CalculateTimeJitter(void); 	// calculate time jitter once dumps are full
void PlotPMF(void); 							// create pmf plot once dumps are full

// size of dumps
const int DUMP_SIZE = 1000;
// counter to increment through dumps
volatile uint32_t Count;
// time dump
volatile uint32_t TimeDump[DUMP_SIZE];
// ADC value dump
volatile uint32_t ADCDump[DUMP_SIZE];


#define PF4   (*((volatile uint32_t *)0x40025040))
	
// Subroutine to wait 10 msec
void DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}

// wait for button input
void Pause(void){
  while(PF4==0x00){ 
    DelayWait10ms(10);
  }
  while(PF4==0x10){
    DelayWait10ms(10);
  }
}
	
// This debug function initializes Timer0A to request interrupts
// at a 100 Hz frequency.  It is similar to FreqMeasure.c.
void Timer0A_Init100HzInt(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 799999;         // start value for 100 Hz interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}	
// This debug function initializes Timer2 to request interrupts
// at around 100 Hz frequency.  It is similar to FreqMeasure.c.
void Timer2_Init(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x04;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER2_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER2_TAILR_R = 7900;         // start value for almost 10 kHz interrupts
  TIMER2_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER2_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer2=priority 1
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x20000000; // top 3 bits
  NVIC_EN0_R = 1<<23;              // enable interrupt 19 in NVIC
}

// Interrupt handler that reads ADC value and stores in dump 
void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
  PF2 ^= 0x04;                	  	// profile
  PF2 ^= 0x04;                  		// profile
	if(Count < DUMP_SIZE) {
		ADCDump[Count] = ADC0_InSeq3();		// Add adc value to dump
		TimeDump[Count] = TIMER1_TAR_R;		// Add time to array
		Count+=1;													// increment counter
	}
  PF2 ^= 0x04;                  		// profile
}

// Timer2 handler to introduce jitter
void Timer2A_Handler(void) {
	TIMER2_ICR_R = 0x01; // acknowledge timer2a timeout
}


// Initializes timer to store value into time dump
void Timer1_Init(void){
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = 0xFFFFFFFF;  // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  //TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 37, interrupt number 21
  //NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}

// initialize GPIO, Timers, and LCD
void init(void) {
  PLL_Init(Bus80MHz);                   // 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x20;            // activate port F
  ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
	Timer1_Init();												// set up Timer1
	//Timer2_Init();												// set up Timer2 for interrupts
  Timer0A_Init100HzInt();               // set up Timer0A for 100 Hz interrupts
  ST7735_InitR(INITR_REDTAB);						// set up LCD
  GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x16;          // disable alt funct on PF2, PF1
	GPIO_PORTF_PUR_R |= 0x10;				 			// pullup for PF4
  GPIO_PORTF_DEN_R |= 0x16;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  PF2 = 0;                      				// turn off LED
  PF1 = 0;                      				// turn off LED
	Count = 0;														// Start Counter at 0
	//EnableInterrupts();
}

// run with hardware averaging
void DisplayWithAveraging(int averagingValue, char* val) {
	ADC0_SAC_R = averagingValue;				// set averaginge value
	ST7735_FillScreen(ST7735_BLACK);		// clear screen
	ST7735_SetCursor(0,0);							// display averaging value
	ST7735_OutString("Averaging: ");
	ST7735_OutString(val);
	ST7735_OutString("\r");
																			// empty array
	for(int i = 0; i <DUMP_SIZE; i += 1) {
		ADCDump[i] = 0;
		TimeDump[i] = 0;
	}				
	Count = 0;
  EnableInterrupts();				
	while(Count < DUMP_SIZE) {					// wait for dumps to fill
		//PF1  = (PF1*12345678)/1234567+0x02;  		// toggle heartbeat
		PF1  ^= 0x02;  					// toggle heartbeat
	}
	DisableInterrupts();								// make sure no more data is being added to dumps
	CalculateTimeJitter();							// Display time jitter
	PlotPMF();													// display pmf
}

void DrawLines(void) {
		ST7735_FillScreen(ST7735_BLACK);					// clear screen
		ST7735_Line(10,100,17,5,ST7735_WHITE);		// draw lines
		ST7735_Line(10,20,100,10,ST7735_WHITE);
		ST7735_Line(10,100,5,5,ST7735_WHITE);
		ST7735_Line(10,20,100,30,ST7735_WHITE);
}

int main(void){
	init();
	
	while(1) {
		DrawLines();
		Pause();
		DisplayWithAveraging(0x00, "x0");					// display pmf with hw averaging
		Pause();
		DisplayWithAveraging(0x02, "x4");
		Pause();
		DisplayWithAveraging(0x04, "x16");
		Pause();
		DisplayWithAveraging(0x06, "x64");
		Pause();
		
	}
}

// Calculates time jitter from dumps and displays to LCD
void CalculateTimeJitter() {	
	uint32_t maxTime = TimeDump[1] - TimeDump[2];			// initialize variables
	uint32_t minTime = TimeDump[1] - TimeDump[2];
	
	for(uint32_t i = 1; i < DUMP_SIZE - 2; i += 1) {					// find each time difference
		uint32_t nextTime = TimeDump[i] - TimeDump[i + 1];			// and compare to current max 
		if(nextTime > maxTime) { maxTime = nextTime; }					// and min
		if(nextTime < minTime) { minTime = nextTime; }
	}
	
	uint32_t timeJitter = (maxTime - minTime)*25/2;						// compute time jitter
	ST7735_OutString("Jitter: ");
	ST7735_OutUDec(timeJitter);
	ST7735_OutString(" ns\r");
}

// size of occurrences array
const uint32_t OCCURRENCES_SIZE = 128;
// stores the occurrences of each adc value in the dump
static uint32_t Occurrences[OCCURRENCES_SIZE];

// Creates and displays the graph of the PMF to the LCD
void PlotPMF() {
	uint32_t max = ADCDump[0];
	uint32_t min = ADCDump[0];
	for(uint32_t i = 1; i < DUMP_SIZE; i += 1) {		// find min and max to scale graph
		if(ADCDump[i] > max) { max = ADCDump[i]; }
		if(ADCDump[i] < min) { min = ADCDump[i]; }
	}
	for(uint32_t i = 0; i < OCCURRENCES_SIZE; i += 1) {
		Occurrences[i] = 0;
	}
	for(uint32_t i = 0; i < DUMP_SIZE; i += 1) {		// compute the number of occurrences 
		uint32_t ADCValue = (ADCDump[i]-min)*128/(max-min+1);		// scaling
		Occurrences[ADCValue] += 1;
	}
	ST7735_OutString("Min: ");							// display min and max
	ST7735_OutUDec(min);
	ST7735_OutString(" Max: ");
	ST7735_OutUDec(max);
	ST7735_PlotClear(0,DUMP_SIZE);
	for(int i = 0; i < OCCURRENCES_SIZE; i += 1) {				// display graph
		ST7735_PlotLine(Occurrences[i]);
		ST7735_PlotNext();
	}
	
}

