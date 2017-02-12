// ******** fixed.c ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 1/25/2017
// Implementations for funtions in fixed.h
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya and Dylan
// Last Revision: 1/29/2017


#include <stdint.h>
#include "ST7735.h"
#include "fixed.h"

const int32_t MAX_DECIMAL = 10000;
const int32_t MIN_DECIMAL = -10000;

/****************ST7735_sDecOut3***************
 converts fixed point number to LCD
 format signed 32-bit with resolution 0.001
 range -9.999 to +9.999
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
 */
void ST7735_sDecOut3(int32_t n) {
	if(n < MAX_DECIMAL && n > MIN_DECIMAL) { // if valid n
		char output[] = "  .   ";
		// if negative, add sign and change to positive
		if(n < 0) { 
			output[0] = '-';
			n *= -1;
		}
		// create output
		for(uint32_t i = 5; i > 2; i--) {
			uint32_t nextDigit = n % 10;
			n = n / 10;
			output[i] = ('0' + nextDigit);
		}
		output[1] = ('0' + n);
		// display
		ST7735_OutString(output);
	} else { 
		// print error if not valid n
		ST7735_OutString(" *.***");
	}
}

const uint32_t MAX_BINARY = 256000;

/**************ST7735_uBinOut8***************
 unsigned 32-bit binary fixed-point with a resolution of 1/256. 
 The full-scale range is from 0 to 999.99. 
 If the integer part is larger than 256000, it signifies an error. 
 The ST7735_uBinOut8 function takes an unsigned 32-bit integer part 
 of the binary fixed-point number and outputs the fixed-point value on the LCD
 Inputs:  unsigned 32-bit integer part of binary fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
*/
void ST7735_uBinOut8(uint32_t n) {
	// if valid n
	if(n < MAX_BINARY) {  
		char output[] = "   .  ";
		// change to fixed point
		n *= 100;
		n = n >> 8;
		// decimal places of output
		for(uint32_t i = 5; i > 3; i--) {
			uint32_t nextDigit = n % 10;
			n = n / 10;
			output[i] = '0' + nextDigit;
		}
		// whole value places of output
		uint32_t i = 2;
		do {
			uint32_t nextDigit = n % 10;
			n = n / 10;
			output[i] = '0' + nextDigit;
			i--;
		} while(n != 0);
		// display output
		ST7735_OutString(output);
	} else { 
		// if not valid n, display stars
		ST7735_OutString("***.**");
	}
}

static int32_t XMin;
static int32_t XMax;
static int32_t YMin;
static int32_t YMax;

/**************ST7735_XYplotInit***************
 Specify the X and Y axes for an x-y scatter plot
 Draw the title and clear the plot area
 Inputs:  title  ASCII string to label the plot, null-termination
          minX   smallest X data value allowed, resolution= 0.001
          maxX   largest X data value allowed, resolution= 0.001
          minY   smallest Y data value allowed, resolution= 0.001
          maxY   largest Y data value allowed, resolution= 0.001
 Outputs: none
 assumes minX < maxX, and miny < maxY
*/
void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, int32_t minY, int32_t maxY) {
	// set screen to black
	ST7735_FillScreen(0);  
	ST7735_SetCursor(0,0);
	// print title
	ST7735_OutString(title); 
	// set globals
	XMin = minX;
	XMax = maxX;
	YMin = minY;
	YMax = maxY;
}

/**************ST7735_XYplot***************
 Plot an array of (x,y) data
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]) {
	for(uint32_t i = 0; i < num; i++) {
		// only if within bounds provided by ST7735_XYplotInit
		if( bufX[i] >= XMin && bufX[i] <= XMax && bufY[i] >= YMin && bufY[i] <= YMax) {
			// change to pixel values
			uint32_t j = (127*(bufX[i]-XMin))/(XMax-XMin);  
			uint32_t k = 32+(127*(YMax-bufY[i]))/(YMax-YMin);
			// display point
			ST7735_DrawPixel(j,k,ST7735_BLUE);
		}

	}
}
