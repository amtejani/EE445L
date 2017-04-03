// ******** Switch.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/20/2017
// switch driver to read from 3 buttons
// 2 on-board, and 1 external
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/20/2017


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

// PF4 connected to a negative logic switch using internal pull-up (trigger on both edges)
#include <stdint.h>
#include "switch.h"
#include "../inc/tm4c123gh6pm.h"
#define PF1                     (*((volatile uint32_t *)0x40025008))
#define PF3											(*((volatile uint32_t *)0x40025020))
#define PF4                     (*((volatile uint32_t *)0x40025040))
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile static unsigned long Last;      // previous
void (*Button1Task)(void);    // user function to be executed on button 1 touch
void (*Button2Task)(void);    // user function to be executed on button 2 touch
void (*MagnetTask)(void);    // user function to be executed on button 3 touch

// arm timer 1 to time 2ms in oneshot mode
static void Timer1Arm(void){
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x0000001;    // 3) 1-SHOT mode
  TIMER1_TAILR_R = 160000-1;    // 4) 2ms reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 21
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER0A
}

// arm gpio interrupts
static void GPIOArm(void){
  GPIO_PORTF_ICR_R = 0x1A;      // (e) clear flag4
  GPIO_PORTF_IM_R |= 0x1A;      // (f) arm interrupt on PF4 *** No IME bit as mentioned in Book ***
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // (g) priority 5
  NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC  
}
// Initialize switch interface on PF4,1,0 
// Inputs:  pointer to a function to call on touch (falling edge),
//          pointer to a function to call on release (rising edge)
// Outputs: none 
void Switch_Init(void(*button1Task)(void), void(*button2Task)(void),
			void(*magnetTask)(void)){
  // **** general initialization ****
  SYSCTL_RCGCGPIO_R |= 0x00000020; // (a) activate clock for port F
  while((SYSCTL_PRGPIO_R & 0x00000020) == 0){};
	GPIO_PORTF_CR_R = 0x1A;           // allow changes to PF4-0
  GPIO_PORTF_DIR_R &= ~0x1A;    // (c) make PF4 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x1A;  //     disable alt funct on PF4,1,0
  GPIO_PORTF_DEN_R |= 0x1A;     //     enable digital I/O on PF4,1,0
  GPIO_PORTF_PCTL_R &= ~0x000FF0F0; // configure PF4 as GPIO
  GPIO_PORTF_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTF_PUR_R |= 0x1A;     //     enable weak pull-up on PF4,1,0
  GPIO_PORTF_IS_R &= ~0x1A;     // (d) PF4 is edge-sensitive
  GPIO_PORTF_IBE_R |= 0x1A;     //     PF4 is both edges
  GPIOArm();

  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER0
	Button1Task = button1Task;		// user defined methods
	Button2Task = button2Task;
	MagnetTask = magnetTask;
  Last = (PF4 | PF3 | PF1);     // initial switch state
 }
// Interrupt on rising or falling edge of PF4 (CCP0)
void GPIOPortF_Handler(void){
  GPIO_PORTF_IM_R &= ~0x1A;     // disarm interrupt on PF4 
														// if last for bit PF_ is 0, execute state change
  if(Last == 0x18){    			// button 3 press, PF4
    (*MagnetTask)(); 			// execute user task
  } else if(Last == 0x12){   // button 2 press, PF1
    (*Button2Task)();  			// execute user task
  } else if(Last == 0x0A){   // button 1 press, PF0
    (*Button1Task)();  			// execute user task
  }
  Timer1Arm(); // start one shot
}
// Interrupt 10 ms after rising edge of PF0,1,4
void Timer1A_Handler(void){
  TIMER1_IMR_R = 0x00000000;    // disarm timeout interrupt
  Last = (PF4 | PF3 | PF1);  // switch state
  GPIOArm();   // start GPIO
}

