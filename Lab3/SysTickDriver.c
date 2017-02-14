// ******** SysTickDriver.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/8/2017
// Creates a clock timer which increments every second
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/9/2017

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

   Program 5.12, section 5.7

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

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "SysTickDriver.h"

#define PF2             (*((volatile uint32_t *)0x40025010))

static uint32_t MilliSeconds;
static uint32_t Seconds;
static uint32_t Minutes;
static uint32_t Hours;
volatile uint32_t AtomicTime;

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// **************SysTick_Init*********************
// Initialize SysTick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns (assuming 50 MHz clock)
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(){long sr;
  sr = StartCritical();
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = 79999;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
                              // enable SysTick with core clock and interrupts
  NVIC_ST_CTRL_R = 0x07;
  GPIO_PORTF_DIR_R |= 0x04;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x4;          // disable alt funct on PF2
	GPIO_PORTF_PUR_R |= 0x4;				 			// pullup for PF4
  GPIO_PORTF_DEN_R |= 0x4;             // enable digital I/O on PF2
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
	Seconds = 0;
	Minutes = 0;
	Hours = 0;
	AtomicTime = 0;
  EndCritical(sr);
}

// **************SysTick_Handler******************
// Interrupt service routine
// increment seconds, minutes, and hours at appropriate time
// Executed every second
void SysTick_Handler(void) {
	PF2 ^= 0x04; // heartbeat
	MilliSeconds = (MilliSeconds + 1) % 1000;
	if(MilliSeconds == 0) {
		Seconds = (Seconds + 1) % 60;		// increment seconds
		if (Seconds == 0) {
			Minutes = (Minutes + 1) % 60;	// increment minutes if 60 seconds have passed	
			if(Minutes == 0) {
				Hours = (Hours + 1) % 24;		// increment hours if 60 minutes have passed
			}
		}
	}
	AtomicTime = Hours*100 + Minutes;
}

// setTime
// sets new time
void SetTime(uint32_t time) {
	Hours = time/100;
	Minutes = time%100;
}


