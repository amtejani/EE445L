// ******** keypad.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 3/29/2017
// Header file for keypad driver
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 3/29/2017


// Initialize keypad interface
// Inputs:  pointer to a function to call on keypad button press
// Outputs: none 
void Keypad_Init(void(*magnetTask)(uint32_t));

// Interrupt on rising or falling edge of keypad button press
// Port D
// Execute button task
void GPIOPortD_Handler(void);

// Interrupt on rising or falling edge of keypad button press
// Port E
// Execute button task
void GPIOPortE_Handler(void);

// Interrupt 10 ms after rising edge of keypad button press
// Set last operation
void Timer0A_Handler(void);
