#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <complex>
#include <vector>
using namespace std;

// Logfile manager library
#include "logfile_manager/logfile_manager.h"

// Port audio library
#include "portaudio.h"

// COMPSK library
#include "cyclic_fifo.h"

#ifndef COMPSK_PORT_AUDIO_DATA_H
#define COMPSK_PORT_AUDIO_DATA_H

class PortAudioData
{
public:
	PortAudioData();
	~PortAudioData();

	/////////////////////
	// Data management //
	/////////////////////

	//Set PortAudio data
	int SetAudioMode(int audio_mode);

	int SetLogFileManager(LogFileManager* logfile_manager);

	//Get PortAudio data
	CyclicFifo* GetAudioInputFifo();
	CyclicFifo* GetAudioOutputFifo();

	int GetAudioMode();

	//Audio modes
	enum
	{
		STANDBY_MODE = 0,
		RECORD_MODE = 1,
		PLAY_MODE = 2,
		PLAYBACK_MODE = 3,
		NOISE_MODE = 4
	};

	//////////////////////
	// COMPSK functions //
	//////////////////////

	int Initialize();

	/////////////////////////
	// Debugging functions //
	/////////////////////////

	int Print();

private:
	////////////////////////////
	// Internal use functions //
	////////////////////////////

	///////////////////
	// Internal data //
	///////////////////

	//Audio data
	CyclicFifo* audio_input_fifo;
	CyclicFifo* audio_output_fifo;

	int audio_mode;

	//Application data
	LogFileManager* logfile_manager;
};

#endif
