// ******** SwitchDriver.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/8/2017
// Driver to interface with speaker
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/9/2017


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
#include "SwitchDriver.h"
#include "../inc/tm4c123gh6pm.h"
#define PF0                     (*((volatile uint32_t *)0x40025004))
#define PF1											(*((volatile uint32_t *)0x40025008))
#define PF3                     (*((volatile uint32_t *)0x40025020))
#define PF4                     (*((volatile uint32_t *)0x40025040))
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile static unsigned long Touch1;     // true on touch button 1
volatile static unsigned long Release1;   // true on release button 1
volatile static unsigned long Touch2;     // true on touch button 2
volatile static unsigned long Release2;   // true on release button 2
volatile static unsigned long Touch3;     // true on touch button 3
volatile static unsigned long Release3;   // true on release button 3
volatile static unsigned long Touch4;     // true on touch button 4
volatile static unsigned long Release4;   // true on release button 4
volatile static unsigned long Last;      // previous
void (*Button1Task)(void);    // user function to be executed on button 1 touch
void (*Button2Task)(void);    // user function to be executed on button 2 touch
void (*Button3Task)(void);    // user function to be executed on button 3 touch
void (*Button4Task)(void);    // user function to be executed on button 4 touch

// arm timer 0 to time 2ms in oneshot mode
static void Timer0Arm(void){
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x0000001;    // 3) 1-SHOT mode
  TIMER0_TAILR_R = 160000-1;    // 4) 2ms reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}

// arm gpio interrupts
static void GPIOArm(void){
  GPIO_PORTF_ICR_R = 0x1B;      // (e) clear flag4
  GPIO_PORTF_IM_R |= 0x1B;      // (f) arm interrupt on PF4 *** No IME bit as mentioned in Book ***
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // (g) priority 5
  NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC  
}
// Initialize switch interface on PF4,3,1,0 
// Inputs:  pointer to a function to call on touch (falling edge),
//          pointer to a function to call on release (rising edge)
// Outputs: none 
void Switch_Init(void(*button1Task)(void), void(*button2Task)(void),
			void(*button3Task)(void), void(*button4Task)(void)){
  // **** general initialization ****
  SYSCTL_RCGCGPIO_R |= 0x00000020; // (a) activate clock for port F
  while((SYSCTL_PRGPIO_R & 0x00000020) == 0){};
	GPIO_PORTF_LOCK_R = 0x4C4F434B;
	GPIO_PORTF_CR_R = 0x1B;           // allow changes to PF4-0
  GPIO_PORTF_DIR_R &= ~0x1B;    // (c) make PF4 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x1B;  //     disable alt funct on PF4,3,1,0
  GPIO_PORTF_DEN_R |= 0x1B;     //     enable digital I/O on PF4,3,1,0
  GPIO_PORTF_PCTL_R &= ~0x000FF0FF; // configure PF4 as GPIO
  GPIO_PORTF_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTF_PUR_R |= 0x1B;     //     enable weak pull-up on PF4-1
  GPIO_PORTF_IS_R &= ~0x1B;     // (d) PF4 is edge-sensitive
  GPIO_PORTF_IBE_R |= 0x1B;     //     PF4 is both edges
  GPIOArm();

  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
	Button1Task = button1Task;		// user defined methods
	Button2Task = button2Task;
	Button3Task = button3Task;
	Button4Task = button4Task;
  Touch1 = 0;                   // touch and release signals
  Release1 = 0;
  Touch2 = 0;   
  Release2 = 0;
  Touch3 = 0;   
  Release3 = 0;
  Touch4 = 0;    
  Release4 = 0;
  Last = (PF4 | PF3 | PF1 | PF0);     // initial switch state
 }
// Interrupt on rising or falling edge of PF4 (CCP0)
void GPIOPortF_Handler(void){
  GPIO_PORTF_IM_R &= ~0x1B;     // disarm interrupt on PF4 
	if(Last & 0x10) {		// used for wait for button presses
		Touch4 = 1;
	} else {
		Release4 = 1;
	}
	if(Last & 0x8) {
		Touch3 = 1;
	} else {
		Release3 = 1;
	} 
	if(Last & 0x2) {
		Touch2 = 1;
	} else if(!(Last & 0x2)) {
		Release2 = 1;
	}
	if(Last & 0x1) {
		Touch1 = 1;
	} else {
		Release1 = 1;
	}
														// if last for bit PF_ is 0, execute state change
  if(Last == 0x0B){    			// button 1 press
    (*Button1Task)(); 			// execute user task
  } else if(Last == 0x13){   // button 2 press
    (*Button2Task)();  			// execute user task
  } else if(Last == 0x19){   // button 3 press
    (*Button3Task)();  			// execute user task
  } else if(Last == 0x1A){   // button 4 press
    (*Button4Task)();  			// execute user task
  }
  Timer0Arm(); // start one shot
}
// Interrupt 10 ms after rising edge of PF4
void Timer0A_Handler(void){
  TIMER0_IMR_R = 0x00000000;    // disarm timeout interrupt
  Last = (PF4 | PF3 | PF1 | PF0);  // switch state
  GPIOArm();   // start GPIO
}

// Wait for switch to be pressed 
// There will be minimum time delay from touch to when this function returns
// Inputs:  none
// Outputs: none 
void Switch1_WaitPress(void){
  while(Touch1==0){}; // wait for press
  Touch1 = 0;  // set up for next time
}

// Wait for switch to be released 
// There will be minimum time delay from release to when this function returns
// Inputs:  none
// Outputs: none 
void Switch1_WaitRelease(void){
  while(Release1==0){}; // wait
  Release1 = 0; // set up for next time
}

// Wait for switch to be pressed 
// There will be minimum time delay from touch to when this function returns
// Inputs:  none
// Outputs: none 
void Switch2_WaitPress(void){
  while(Touch2==0){}; // wait for press
  Touch2 = 0;  // set up for next time
}

// Wait for switch to be released 
// There will be minimum time delay from release to when this function returns
// Inputs:  none
// Outputs: none 
void Switch2_WaitRelease(void){
  while(Release2==0){}; // wait
  Release2 = 0; // set up for next time
}

// Wait for switch to be pressed 
// There will be minimum time delay from touch to when this function returns
// Inputs:  none
// Outputs: none 
void Switch3_WaitPress(void){
  while(Touch3==0){}; // wait for press
  Touch3 = 0;  // set up for next time
}

// Wait for switch to be released 
// There will be minimum time delay from release to when this function returns
// Inputs:  none
// Outputs: none 
void Switch3_WaitRelease(void){
  while(Release3==0){}; // wait
  Release3 = 0; // set up for next time
}

// Wait for switch to be pressed 
// There will be minimum time delay from touch to when this function returns
// Inputs:  none
// Outputs: none 
void Switch4_WaitPress(void){
  while(Touch4==0){}; // wait for press
  Touch4 = 0;  // set up for next time
}

// Wait for switch to be released 
// There will be minimum time delay from release to when this function returns
// Inputs:  none
// Outputs: none 
void Switch4_WaitRelease(void){
  while(Release4==0){}; // wait
  Release4 = 0; // set up for next time
}
