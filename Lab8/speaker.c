// ******** SpeakerDriver.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/8/2017
// Driver to interface with speaker
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/9/2017

#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>
#include "speaker.h"

#define PF0                     (*((volatile uint32_t *)0x40025004))

void DisableInterrupts(void); 		// Disable interrupts
void EnableInterrupts(void);  		// Enable interrupts
long StartCritical (void);    		// previous I bit, disable interrupts
void EndCritical(long sr);    		// restore I bit to previous value
void WaitForInterrupt(void);  		// low power mode

// starts timer 2 and interrupts
// sets timer to play A note
static void Timer2_Arm(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x04;      // activate timer2
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN; // disable timer2A during setup
  TIMER2_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER2_TAILR_R = 90908;         // start value for 880 Hz interrupts
  TIMER2_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER2_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer2=priority 4
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // top 3 bits
  NVIC_EN0_R = 1<<23;              // enable interrupt 19 in NVIC
}

// initialize PE5 to output pulse to speaker
// start timer
void Speaker_Init(void) {
  volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R  |= 0x00000020;        // enable port E
	delay               = SYSCTL_RCGCGPIO_R;
	GPIO_PORTF_DIR_R   |= 0x01;              // Make PE5 in
	GPIO_PORTF_AFSEL_R &= ~0x01;             // Disable Alternate Function on PE5
	GPIO_PORTF_DEN_R   |= 0x01;              // Enable digital I/O for PE5
	GPIO_PORTF_AMSEL_R &= ~0x01;             // Disable analog functionality
	Timer2_Arm();
}


// toggles value at PE5, creating square wave
void Timer2A_Handler(void) {
	TIMER2_ICR_R = 0x01; // acknowledge timer2a timeout
	PF0 ^= 0x20;
}
