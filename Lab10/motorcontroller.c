// ******** motorcontroller.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 4/12/2017
// Correct error of motor output
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 4/12/2017

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "motor.h"
#include "tach.h"

void DisableInterrupts(void); 		// Disable interrupts
void EnableInterrupts(void);  		// Enable interrupts
long StartCritical (void);    		// previous I bit, disable interrupts
void EndCritical(long sr);    		// restore I bit to previous value
void WaitForInterrupt(void);  		// low power mode

volatile int32_t DesiredSpeed;
//uint32_t Period; // 24-bit, 12.5 ns units
volatile int32_t Speed; // motor speed in 0.1 rps
int32_t E; // speed error in 0.1 rps
int32_t U; // duty cycle 40 to 39960
uint32_t Count;


// starts timer 2 and interrupts
// sets timer to play A note
static void Timer2_Arm(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x04;      // activate timer2
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER2_CTL_R &= ~TIMER_CTL_TAEN; // disable timer2A during setup
  TIMER2_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER2_TAILR_R = 800000;         // start value for 100 Hz interrupts
  TIMER2_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER2_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer2=priority 4
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // top 3 bits
  NVIC_EN0_R = 1<<23;              // enable interrupt 19 in NVIC
	EnableInterrupts();
}

void MotorConrollerInit(void) {
	Timer2_Arm();
	PWM0B_Init(40000,30000);
	PeriodMeasure_Init();
	DesiredSpeed = 250;
	U = 30000;
	Count = 0;
}

void Timer2A_Handler(void){
	TIMER2_ICR_R = 0x01; // acknowledge timer2A timeout
	uint32_t Period = GetPeriod();
	if (Count < 5 && Done) {
		Speed = 200000000/Period; // 0.1 rps
		E = DesiredSpeed-Speed; // 0.1 rps
		U = U+(10*E)/64; // discrete integral
		if(U < 40) U=40; // Constrain output
		if(U>39960) U=39960; // 40 to 39960
		PWM0B_Duty(U); // output
		Count = 0;
		Done = 0;
	} else if (Count == 5) {
		Speed = 0;
		if(DesiredSpeed == 0) {
			PWM0B_Duty(0); // output
		} else {
			PWM0B_Duty(30000); // output
		}
		Count = 0;
	} else {
		Count++;
	}
}

// increase output by 5rps
void IncrementDutyCycle() {
	DesiredSpeed += 50;
	if(DesiredSpeed > 400) DesiredSpeed = 400;
}
// increase output by 5rps
void DecrementDutyCycle() {
	DesiredSpeed -= 50;
	if(DesiredSpeed < 0) DesiredSpeed = 0;
}
