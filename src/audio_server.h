#include <stdlib.h>
#include <stdio.h>

#include <complex>
#include <vector>
using namespace std;

// Logfile manager library
#include "logfile_manager/logfile_manager.h"

// Port audio library
#include "portaudio.h"

// COMPSK library
#include "cyclic_fifo.h"
#include "port_audio_data.h"

#ifndef COMPSK_AUDIO_SERVER_H
#define COMPSK_AUDIO_SERVER_H

#define CHANNEL_COUNT 2

int audio_server_callback(const void* input_buffer, void* output_buffer, unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, void* callback_data);
void audio_server_finished(void* callback_data);

class AudioServer
{
public:
	AudioServer();
	~AudioServer();

	/////////////////////
	// Data management //
	/////////////////////

	//Set AudioServer data
	int SetLogFileManager(LogFileManager* logfile_manager);

	//Get AudioServer data
	PortAudioData* GetPortAudioData();

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

	int InitializePortAudioData();
	int InitializePortAudio();

	int ClosePortAudioStream();

	///////////////////////
	// Internal use data //
	///////////////////////

	//Port audio data
	PaStreamParameters* pa_stream_parameters;
	PaStream* pa_stream;

	PortAudioData* port_audio_data;

	//Logfile manager
	LogFileManager* logfile_manager;
};

#endif

