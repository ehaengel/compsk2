#include <stdlib.h>
#include <stdio.h>

#include <complex>
#include <vector>
using namespace std;

// COMPSK Library
#include "cyclic_fifo.h"
#include "audio_server.h"

#ifndef COMPSK_SIGNAL_MODULATOR_H
#define COMPSK_SIGNAL_MODULATOR_H

class SignalModulator
{
public:
	SignalModulator();
	~SignalModulator();

	/////////////////////
	// Data management //
	/////////////////////

	//Set SignalModulator data
	int SetPortAudioData(PortAudioData* port_audio_data);
	int SetLogFileManager(LogFileManager* logfile_manager);

	//Get SignalModulator data

	//////////////////////
	// COMPSK functions //
	//////////////////////

	int Initialize();

	int ModulateCharacterData(char c);
	int ModulateStringData(string message);

	int TransmitSynchronizationSignal();

	/////////////////////////
	// Debugging functions //
	/////////////////////////

	int Print();

private:
	////////////////////////////
	// Internal use functions //
	////////////////////////////

	int CreateSynchronizationSignal();

	///////////////////////
	// Internal use data //
	///////////////////////

	//Modulation data
	int symbol_period;
	int symbol_length;

	string synch_signal_message;
	vector< complex<double> > synch_signal_vector;

	//Application data
	PortAudioData* port_audio_data;
	LogFileManager* logfile_manager;
};

#endif

