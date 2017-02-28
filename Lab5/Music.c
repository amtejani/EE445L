// ******** Music.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/20/2017
// music player functions
// play, pause, and rewind
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/28/2017

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Music.h"
#include "DAC.h"
#include "SysTickInts.h"
#include "Timer1.h"
#include "Timer2.h"
#include "Timer3.h"

uint16_t Wave[64] = {  
  1024,1122,1219,1314,1407,1495,1580,1658,1731,1797,1855,
  1906,1948,1981,2005,2019,2024,2019,2005,1981,1948,1906,
  1855,1797,1731,1658,1580,1495,1407,1314,1219,1122,1024,
  926,829,734,641,553,468,390,317,251,193,142,
  100,67,43,29,24,29,43,67,100,142,193,
  251,317,390,468,553,641,734,829,926
}; 
uint16_t Oboe64[64]={
1024, 1024, 1014, 1008, 1022, 1065, 1093, 1006, 858, 711, 612, 596, 672, 806, 952, 1074, 1154, 1191, 
1202, 1216, 1236, 1255, 1272, 1302, 1318, 1299, 1238, 1140, 1022, 910, 827, 779, 758, 757, 782, 856, 
972, 1088, 1177, 1226, 1232, 1203, 1157, 1110, 1067, 1028, 993, 958, 929, 905, 892, 900, 940, 1022, 
1125, 1157, 1087, 965, 836, 783, 816, 895, 971, 1017};

uint16_t Bassoon64[64]={
1068, 1169, 1175, 1161, 1130, 1113, 1102, 1076, 1032, 985, 963, 987, 1082, 1343, 1737, 1863, 
1575, 1031, 538, 309, 330, 472, 626, 807, 1038, 1270, 1420, 1461, 1375, 1201, 1005, 819, 658, 
532, 496, 594, 804, 1055, 1248, 1323, 1233, 1049, 895, 826, 826, 850, 862, 861, 899, 961, 1006, 
1023, 1046, 1092, 1177, 1224, 1186, 1133, 1098, 1102, 1109, 1076, 1027, 1003};

uint16_t Trumpet64[64]={
  987, 1049, 1090, 1110, 1134, 1160, 1139, 1092, 1070, 1042, 1035, 1029, 1008, 1066, 1150, 1170, 1087, 915, 679, 372, 151, 
  558, 1014, 1245, 1260, 1145, 1063, 984, 934, 960, 1027, 1077, 1081, 1074, 1064, 1042, 1010, 974, 968, 974, 994, 1039, 
  1094, 1129, 1125, 1092, 1056, 1056, 1082, 1059, 1046, 1058, 1061, 1045, 1034, 1050, 1094, 1112, 1092, 1063, 1053, 1065, 1052, 992};

static volatile uint32_t eighthBeatLength = 17000000;

static song* S;
// increments through song
static volatile uint32_t SongCounter;
// output value for track 1
static volatile uint32_t Track1Value;
// increments through sine table at proper frequency
static volatile uint32_t Track1Counter;
// counts number of consecutive notes
static volatile uint32_t Track1NoteCounter;
// output value for track 2
static volatile uint32_t Track2Value;
// increments through sine table at proper frequency
static volatile uint32_t Track2Counter;
// counts number of consecutive notes
static volatile uint32_t Track2NoteCounter;
static volatile uint16_t* WaveInstrument;

const uint32_t envelope = 1;


// Read from sine table at Track1 Frequency
void Track1_Read(void) {
	Track1Counter = (Track1Counter+1) % 64;
	uint32_t out = WaveInstrument[Track1Counter];								// get value from sine table
	if(envelope) {
		uint32_t time = eighthBeatLength - TIMER3_TAR_R + Track1NoteCounter * eighthBeatLength;
		if(time < eighthBeatLength/8) {
			out = out * (time) / (eighthBeatLength/8);		// start envelope
		} else {
			out = out * (eighthBeatLength/8) / time;			// end envelope
		}
	}
	Track1Value = out;
}

// Read from sine table at Track2 Frequency
void Track2_Read(void) {
	Track2Counter = (Track2Counter+1) % 64;
	uint32_t out = WaveInstrument[Track2Counter];									// get value from sine table
	if(envelope) {
		uint32_t time = eighthBeatLength - TIMER3_TAR_R + Track2NoteCounter * eighthBeatLength;
		if(time < eighthBeatLength / 8) {
			out = out * (time) / (eighthBeatLength / 8);		// start envelope
		} else {
			out = out * (eighthBeatLength / 8) / time;			// end envelope
		}
	}
	Track2Value = out;
}

// Start next note
// Executes every 1/8th beat
void StartNewNote(void) {
	SongCounter++;
	if((*S).durations1[SongCounter] == 0) {				// if no note playing, turn off timer
		// turn off timer
		TIMER1_CTL_R = 0x00;
		Track1Value = 0;
		Track1NoteCounter = 0;
	} else if(++Track1NoteCounter >= (*S).durations1[SongCounter-1]) {
																								// if new note, reset values, start timer 
		TIMER1_CTL_R = 0x01;
		Track1NoteCounter = 0;
		Track1Counter = 0;
	}
	Timer1_Period((*S).track1[SongCounter]);
	if((*S).durations2[SongCounter] == 0) {				// if no note playing, turn off timer
		TIMER2_CTL_R = 0x00;
		Track2Value = 0;
		Track2NoteCounter = 0;
		// turn off timer
	} else if(++Track2NoteCounter >= (*S).durations2[SongCounter-1]) {
																								// if new note, reset values, start timer 
		TIMER2_CTL_R = 0x01;
		Track2NoteCounter = 0;
		Track2Counter = 0;
	}
	Timer2_Period((*S).track2[SongCounter]);
	if(SongCounter - 1 >= (*S).length) 
		Music_Rewind();
}

// ************ Music_Init ***************
// Sets up music play
void Music_Init(song* s) {
	S = s;
	SongCounter = 0;
	// init dac
	DAC_Init();
	// init systic for outputing dac
	SysTick_Init(1420);
	// init timer1,2 for track1,2
	Timer1_Init(Track1_Read,(*S).track1[SongCounter]);
	Timer2_Init(Track2_Read,(*S).track2[SongCounter]);
	// init timer3 for note duration
	Timer3_Init(StartNewNote,eighthBeatLength);
	// reset values
	Track1Value = 0;
	Track1NoteCounter = 0;
	Track1Counter = 0;
	Track2Value = 0;
	Track2NoteCounter = 0;
	Track2Counter = 0;
	WaveInstrument = Wave;
	// turn off interrupts
	Music_Rewind();
}

static volatile uint32_t Play;
// Toggle between play and pause functionality on button
void PlayPause(void) {
	if(Play) {
		Play = 0;
		Music_Pause();
	} else {
		Play = 1;
		Music_Play();
	}
}

static volatile uint32_t Tempo = 0;
// Change tempo of music
void ChangeTempo(void) {
	Tempo = (Tempo + 1) % 3;
	if(Tempo == 0) {
		eighthBeatLength = 10000000;
	} else if(Tempo == 1) {
		eighthBeatLength = 13000000;
	} else if(Tempo == 2) {
		eighthBeatLength = 8000000;
	}  
	Timer3_Period(eighthBeatLength);
}

static volatile uint32_t Instrument = 0;
// Change tempo of music
void ChangeInstrument(void) {
	Instrument = (Instrument + 1) % 4;
	if(Instrument == 0) {
		WaveInstrument = Wave;
	} else if(Instrument == 1) {
		WaveInstrument = Oboe64;
	} else if(Instrument == 2) {
		WaveInstrument = Bassoon64;
	}  else if(Instrument == 3) {
		WaveInstrument = Trumpet64;
	}  
}

// ************ Music_Play ***************
// Sets up music to begin playing
void Music_Play(void) {
	// turn on systic/timer interrupts
	TIMER1_CTL_R = 0x01;        // enable Timers
	TIMER2_CTL_R = 0x01;
	TIMER3_CTL_R = 0x01;
  NVIC_ST_CTRL_R = 0x07;      // enable SysTick
}

// ************ Music_Pause **************
// Sets up music to stop playing
void Music_Pause(void) {
	// turn off systic/timer interrupts
	TIMER1_CTL_R = 0x00;        // disable Timers
	TIMER2_CTL_R = 0x00;
	TIMER3_CTL_R = 0x00;
  NVIC_ST_CTRL_R = 0;         // disable SysTick
}

// ************ Music_Rewind *************
// Starts from beginning, paused
void Music_Rewind(void) {
	// turn off systic/timer interrupts
	Play = 0;
	TIMER1_CTL_R = 0x00;        // enable Timers
	TIMER2_CTL_R = 0x00;
	TIMER3_CTL_R = 0x00;
  NVIC_ST_CTRL_R = 0;         // disable SysTick
	SongCounter = 0;
	
	// reset counters
}

// Interrupt service routine
// Executed every 80000000/880/64 cycles(period)
void SysTick_Handler(void){
	uint32_t out = 0;
	if(Track1Value != 0 && Track2Value != 0)
		out += Track1Value/2 + Track2Value/2;
	else if (Track1Value != 0) 
		out += Track1Value;
	else if (Track2Value != 0)
		out += Track2Value;
	DAC_Out(out);
}
