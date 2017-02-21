// ******** DAC.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/20/2017
// Header file for DAC driver
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/20/2017

// ************ DAC_Init ***************
// Initializes SSI1 for DAC output
void DAC_Init(void);

// ************ DAC_Out ****************
// Outputs value to DAC
// Inputs:	12 bit number to be sent to DAC
void DAC_Init(uint32_t out);


