// ******** speaker.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 3/29/2017
// Driver to interface with speaker
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 3/29/2017


#ifndef __SPEAKERDRIVER_H__ // do not include more than once
#define __SPEAKERDRIVER_H__

// initialize PF0 to output pulse to speaker
// start timer to play A note
void Speaker_Init(void);

// toggles value at PE5, creating square wave
void Timer2A_Handler(void);


#endif // __SPEAKERDRIVER_H__
