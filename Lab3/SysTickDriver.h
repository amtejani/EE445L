// ******** SysTickDriver.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/8/2017
// Creates a clock timer which increments every second
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/9/2017


#ifndef __SYSTICKDRIVER_H__ // do not include more than once
#define __SYSTICKDRIVER_H__

// **************SysTick_Init*********************
// Initialize Systick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns (assuming 80 MHz clock)
//        Maximum is 2^24-1
//        Minimum is determined by lenght of ISR
// Output: none
void SysTick_Init(void);

// **************SysTick_Handler******************
// Interrupt service routine
// increment seconds, minutes, and hours at appropriate time
// Executed every second
void SysTick_Handler(void);

#endif // __SYSTICKINTS_H__
