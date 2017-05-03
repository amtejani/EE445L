// ******** FSM.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 4/19/2017
// defines system interface
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 4/19/2017

extern uint32_t State;
// Updates the state of the system
void ChangeState(void);

// Sends status to server
void UpdateState(void);

// Initializes system
void System_Init(void);
