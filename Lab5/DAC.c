// ******** DAC.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/20/2017
// Initialize DAC driver
// And provide function to send value to DAC
// Runs on TM4C123
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/20/2017

#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>
#include "DAC.h"

// ************ DAC_Init ***************
// Initializes SSI1 for DAC output
void DAC_Init(void) {
	SYSCTL_RCGCSSI_R |= 0x02;       // activate SSI1
  SYSCTL_RCGCGPIO_R |= 0x08;      // activate port D
  while((SYSCTL_PRGPIO_R&0x08) == 0){};// ready?
  GPIO_PORTD_AFSEL_R |= 0xB;     // enable alt funct on PD0,1,3
  GPIO_PORTD_DEN_R |= 0xB;       // configure PD0,1,3 as SSI
  GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFF0F00)+0x002022;
  GPIO_PORTD_AMSEL_R = 0;         // disable analog functionality on PD
  SSI1_CR1_R = 0x00000000;        // disable SSI, master mode
  SSI1_CPSR_R = 0x02;             // 8 MHz SSIClk 
  SSI1_CR0_R &= ~(0x0000FFF0);    // SCR = 0, SPH = 0, SPO = 0 Freescale
  SSI1_CR0_R |= 0x0F;             // DSS = 16-bit data
  SSI1_CR1_R |= 0x00000002;       // enable SSI

}

// ************ DAC_Out ****************
// Outputs value to DAC
// Inputs:	12 bit number to be sent to DAC
void DAC_Out(uint32_t out) {
  while((SSI1_SR_R&0x00000002)==0){};// wait until room in FIFO
  SSI1_DR_R = out; // data out
}


