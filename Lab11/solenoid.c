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
#include "solenoid.h"

#define PF2                     (*((volatile uint32_t *)0x40025010))

void DisableInterrupts(void); 		// Disable interrupts
void EnableInterrupts(void);  		// Enable interrupts
long StartCritical (void);    		// previous I bit, disable interrupts
void EndCritical(long sr);    		// restore I bit to previous value
void WaitForInterrupt(void);  		// low power mode

// initialize PE5 to output pulse to speaker
// start timer
void Solenoid_Init(void) {
  volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R  |= 0x00000020;        // enable port E
	delay               = SYSCTL_RCGCGPIO_R;
	GPIO_PORTF_DIR_R   |= 0x04;              // Make PE5 in
	GPIO_PORTF_AFSEL_R &= ~0x04;             // Disable Alternate Function on PE5
	GPIO_PORTF_DEN_R   |= 0x04;              // Enable digital I/O for PE5
	GPIO_PORTF_AMSEL_R &= ~0x04;             // Disable analog functionality
}


// toggles value at PE5, creating square wave
void Solenoid_Out(uint32_t in) {
	PF2 ^= 0x4;
}
