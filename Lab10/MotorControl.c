
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "PLL.h"
#include "switch.h"
#include "motorcontroller.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// Subroutine to wait 10 msec
void DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}
void Display() {
	DelayWait10ms(1);
	ST7735_PlotPoint(Speed, ST7735_BLUE);  // Measured temperature
	ST7735_PlotPoint(DesiredSpeed, ST7735_RED);  // Measured temperature
	ST7735_PlotNextErase();
	ST7735_SetCursor(0,1); ST7735_OutString("RPS M=         ");
	ST7735_SetCursor(0,2); ST7735_OutString("PRS A=         ");
	uint32_t speed = Speed; uint32_t desiredSpeed = DesiredSpeed;
	ST7735_SetCursor(7,1); ST7735_OutUDec(speed/10); ST7735_OutChar('.'); ST7735_OutUDec(speed%10);
	ST7735_SetCursor(7,2); ST7735_OutUDec(desiredSpeed/10); ST7735_OutChar('.'); ST7735_OutUDec(desiredSpeed%10);
}
int main(void) {
	PLL_Init(Bus80MHz);				// init modules
	ST7735_InitR(INITR_REDTAB);
	Switch_Init(IncrementDutyCycle,DecrementDutyCycle);
	MotorConrollerInit();
	
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_SetCursor(0,0); ST7735_OutString("Lab 10: Motor");
  ST7735_PlotClear(0,400);  // range from 0 to 4095
	while(1){
		//WaitForInterrupt();
		Display();
	}
}
