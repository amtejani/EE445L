// ******** solenoid.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 3/29/2017
// Driver to interface with solenoid
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 3/29/2017


#ifndef __SOLENOIDDRIVER_H__ // do not include more than once
#define __SOLENOIDDRIVER_H__

// initialize PF0 to output pulse to speaker
// start timer to play A note
void Solenoid_Init(void);

// sets value of PF0, activating/deactivation solenoid
void Solenoid_Out(uint32_t);


#endif // __SOLENOIDDRIVER_H__
