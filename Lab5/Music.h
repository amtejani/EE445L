// ******** Music.h ************** 
// Ali Tejani and Caroline Yao
// amt3639 and chy253
// Creation Date: 2/20/2017
// Header file for music player functions
// play, pause, and rewind
// Lab section: Tue/Thur 12:30 - 2 PM
// TA: Lavanya
// Last Revision: 2/20/2017

// note struct
typedef struct {
	uint32_t* reloadValue;
	uint32_t* duration;
} note;

// song struct
typedef struct {
	uint32_t length;
	uint32_t* track1;
	uint32_t* durations1;
	uint32_t* track2;
	uint32_t* durations2;
	//note* track1;
	//note* track2;
} song;

// Toggle between play and pause functionality on button
void PlayPause(void);
// Change tempo of music
void ChangeTempo(void);
// Change Instrument
void ChangeInstrument(void);


// ************ Music_Init ***************
// Sets up music play
void Music_Init(song* s);

// ************ Music_Play ***************
// Begins playing music
void Music_Play(void);

// ************ Music_Pause **************
// Pauses music
void Music_Pause(void);

// ************ Music_Rewind **************
// restarts music from beginning
void Music_Rewind(void);
