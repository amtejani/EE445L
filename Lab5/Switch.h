// ******** Switch.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/20/2017
// Header file for switch driver
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/20/2017


// Initialize switch interface on PF0,1,4
// Inputs:  pointer to a function to call on PF0 touch,
// 			pointer to a function to call on PF1 touch,
// 			pointer to a function to call on PF4 touch
// Outputs: none 
void Switch_Init(void(*button1Task)(void), void(*button2Task)(void),
			void(*button3Task)(void));

// Interrupt on rising or falling edge of PF0,1,4 (CCP0)
// Execute button task
void GPIOPortF_Handler(void);

// Interrupt 10 ms after rising edge of PF0,1,4
// Set last operation
void Timer0A_Handler(void);
