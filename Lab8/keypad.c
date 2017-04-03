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
#include "keypad.h"
#include "../inc/tm4c123gh6pm.h"
#define PD                     (*((volatile uint32_t *)0x400073FC))
#define PE										 (*((volatile uint32_t *)0x4002403C))
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile static unsigned long Last;      // previous
void (*KeypadTask)(uint32_t);    // user function to be executed on button 1 touch

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

// arm gpio port D and E interrupts
static void GPIOArm(void){
  GPIO_PORTE_ICR_R = 0xF;      // (e) clear flag4
  GPIO_PORTE_IM_R |= 0xF;      // (f) arm interrupt on PE *** No IME bit as mentioned in Book ***
  GPIO_PORTD_ICR_R = 0xFF;      // (e) clear flag4
  GPIO_PORTD_IM_R |= 0xFF;      // (f) arm interrupt on PE *** No IME bit as mentioned in Book ***
  NVIC_PRI0_R = (NVIC_PRI0_R&0x00FFFFFF)|0xA0000000; // (g) priority 5
  NVIC_EN0_R = 1<<3;      // (h) enable interrupt 30 in NVIC  
}
// Initialize switch interface on PF4,1,0 
// Inputs:  pointer to a function to call on touch (falling edge),
//          pointer to a function to call on release (rising edge)
// Outputs: none 
void Keypad_Init(void(*keypadTask)(uint32_t)){
  // **** general initialization ****
  SYSCTL_RCGCGPIO_R |= 0x00000018; // (a) activate clock for port D,E
  while((SYSCTL_PRGPIO_R & 0x00000018) == 0){};
	GPIO_PORTD_LOCK_R = 0x4C4F434B;
  GPIO_PORTD_CR_R = 0xFF;           // allow changes to PF4-0
  GPIO_PORTD_DIR_R &= ~0xFF;    // (c) make PF4 in (built-in button)
  GPIO_PORTD_AFSEL_R &= ~0xFF;  //     disable alt funct on PF4,1,0
  GPIO_PORTD_DEN_R |= 0xFF;     //     enable digital I/O on PF4,1,0
  GPIO_PORTD_PCTL_R &= ~0xFFFFFFFF; // configure PF4 as GPIO
  GPIO_PORTD_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTD_PUR_R |= 0xFF;     //     enable weak pull-up on PF4,1,0
  GPIO_PORTD_IS_R &= ~0xFF;     // (d) PF4 is edge-sensitive
  GPIO_PORTD_IBE_R |= 0xFF;     //     PF4 is both edges
  
  GPIO_PORTE_CR_R = 0xF;           // allow changes to PF4-0
  GPIO_PORTE_DIR_R &= ~0xF;    // (c) make PF4 in (built-in button)
  GPIO_PORTE_AFSEL_R &= ~0xF;  //     disable alt funct on PF4,1,0
  GPIO_PORTE_DEN_R |= 0xF;     //     enable digital I/O on PF4,1,0
  GPIO_PORTE_PCTL_R &= ~0xFFFF; // configure PF4 as GPIO
  GPIO_PORTE_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTE_PUR_R |= 0xF;     //     enable weak pull-up on PF4,1,0
  GPIO_PORTE_IS_R &= ~0xF;     // (d) PF4 is edge-sensitive
  GPIO_PORTE_IBE_R |= 0xF;     //     PF4 is both edges
  GPIOArm();

  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  KeypadTask = keypadTask;		// user defined methods
  Last = (0xFF & PD) + ((0xF & PE) << 8);     // initial switch state
 }

void Keypad_Handler(void) {
	switch(Last) {
		case 0xFFF & ~0x001:
			(*KeypadTask)(1);
			break;
		case 0xFFF & ~0x002:
			(*KeypadTask)(4);
			break;
		case 0xFFF & ~0x004:
			(*KeypadTask)(7);
			break;
		case 0xFFF & ~0x008:
			(*KeypadTask)(10);
			break;
		case 0xFFF & ~0x010:
			(*KeypadTask)(2);
			break;
		case 0xFFF & ~0x020:
			(*KeypadTask)(5);
			break;
		case 0xFFF & ~0x040:
			(*KeypadTask)(8);
			break;
		case 0xFFF & ~0x080:
			(*KeypadTask)(0);
			break;
		case 0xFFF & ~0x100:
			(*KeypadTask)(3);
			break;
		case 0xFFF & ~0x200:
			(*KeypadTask)(6);
			break;
		case 0xFFF & ~0x400:
			(*KeypadTask)(9);
			break;
		case 0xFFF & ~0x800:
			(*KeypadTask)(11);
			break;
	}
}
// Interrupt on rising or falling edge of PF4 (CCP0)
void GPIOPortD_Handler(void){
  GPIO_PORTD_IM_R &= ~0xFF;     // disarm interrupt on PD
	Keypad_Handler();
  Timer0Arm(); // start one shot
}
// Interrupt on rising or falling edge of PF4 (CCP0)
void GPIOPortE_Handler(void){
  GPIO_PORTE_IM_R &= ~0xF;     // disarm interrupt on PE
	Keypad_Handler();
  Timer0Arm(); // start one shot
}
// Interrupt 10 ms after rising edge of PF0,1,4
void Timer0A_Handler(void){
  TIMER0_IMR_R = 0x00000000;    // disarm timeout interrupt
  Last = (0xFF & PD) + ((0xF & PE) << 8);  // switch state
  GPIOArm();   // start GPIO
}
