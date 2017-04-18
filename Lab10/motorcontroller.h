// ******** motorcontroller.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 4/12/2017
// Correct error of motor output
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 4/12/2017

extern int32_t Speed;
extern int32_t DesiredSpeed;


// Initialize motor, tachometer, and controller
void MotorConrollerInit(void);

// Initialize timer to correct error
void Timer2A_Handler(void);

// Increase motor output by 5rps
void IncrementDutyCycle(void);

// Decrease motor output by 5rps
void DecrementDutyCycle(void);
