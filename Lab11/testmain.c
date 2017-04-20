
#include <stdint.h>
#include "PLL.h"
#include "switch.h"
#include "keypad.h"
#include "speaker.h"
#include "solenoid.h"
#include "ST7735.h"
uint32_t KeypadPresses[1000] = 	{13};
uint32_t KeypadCounter = 0;
uint32_t LastKey = 13;

char Switch = ' ';

void KeypadPress(uint32_t pressed) {
	KeypadPresses[KeypadCounter++] = pressed;
	LastKey = pressed;
}
void MagnetPress() {
	Switch = 'M';
}
void Button1Press() {
	Switch = '1';
}
void Button2Press() {
	Switch = '2';
}

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

int main1(void) {
	PLL_Init(Bus80MHz);				// init modules
	ST7735_InitR(INITR_REDTAB);
	Switch_Init(Button1Press,Button2Press,MagnetPress);
	Keypad_Init(KeypadPress);
	//Speaker_Init();
	while(1){
		DelayWait10ms(100);
		ST7735_SetCursor(0,0);
		ST7735_FillScreen(ST7735_BLACK);
		ST7735_OutUDec(LastKey);
		ST7735_OutChar('\r');
		ST7735_OutChar(Switch);
	}
	
}
