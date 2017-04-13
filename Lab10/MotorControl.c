
#include <stdint.h>
#include "motor.h"
#include "tach.h"
#include "ST7735.h"
#include "PLL.h"
#include "switch.h"

uint32_t DutyCycle;

// increase output by 5rps
void IncrementDutyCycle() {
}
// increase output by 5rps
void DecrementDutyCycle() {
}

int main(void) {
	PLL_Init(Bus80MHz);				// init modules
	Switch_Init(IncrementDutyCycle,DecrementDutyCycle);
	PWM0B_Init(40000,30000);
	PeriodMeasure_Init();
	while(1){}
}