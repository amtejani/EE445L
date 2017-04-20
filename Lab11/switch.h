// ******** switch.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 3/29/2017
// Header file for magnet and switch drivers
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 3/29/2017


// Initialize switch interface on PF1,3,4
// Inputs:  pointer to a function to call on PF3 touch,
// 			pointer to a function to call on PF4 touch
// Outputs: none 
void Switch_Init(void(*button1Task)(void), void(*button2Task)(void),
					void(*magnetTask)(void));

// Interrupt on rising or falling edge of PF1,3,4 (CCP0)
// Execute button task
void GPIOPortF_Handler(void);

// Interrupt 10 ms after rising edge of PF1,3,4
// Set last operation
void Timer1A_Handler(void);

// Returns status of magnet, open or closed
uint32_t GetDoorStatus(void);
