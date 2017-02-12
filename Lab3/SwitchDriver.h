// ******** SwitchDriver.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/8/2017
// Driver to interface with speaker
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/9/2017


// Initialize switch interface on PF4 
// Inputs:  pointer to a function to call on touch (falling edge),
//          pointer to a function to call on release (rising edge)
// Outputs: none 
void Switch_Init(void(*button1Task)(void), void(*button2Task)(void),
			void(*button3Task)(void), void(*button4Task)(void));

// Interrupt on rising or falling edge of PF4 (CCP0)
void GPIOPortF_Handler(void);

// Interrupt 10 ms after rising edge of PF4
void Timer0A_Handler(void);

// Wait for switch to be pressed 
// There will be minimum time delay from touch to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitPress1(void);

// Wait for switch to be released 
// There will be minimum time delay from release to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitRelease1(void);

// Wait for switch to be pressed 
// There will be minimum time delay from touch to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitPress2(void);

// Wait for switch to be released 
// There will be minimum time delay from release to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitRelease2(void);

// Wait for switch to be pressed 
// There will be minimum time delay from touch to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitPress3(void);

// Wait for switch to be released 
// There will be minimum time delay from release to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitRelease3(void);

// Wait for switch to be pressed 
// There will be minimum time delay from touch to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitPress4(void);

// Wait for switch to be released 
// There will be minimum time delay from release to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitRelease4(void);
