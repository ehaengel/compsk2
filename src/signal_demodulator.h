#include <stdlib.h>
#include <stdio.h>

#include <complex>
#include <vector>
using namespace std;

// COMPSK Library
#include "cyclic_fifo.h"
#include "audio_server.h"

#ifndef COMPSK_SIGNAL_DEMODULATOR_H
#define COMPSK_SIGNAL_DEMODULATOR_H

class SignalDemodulator
{
public:
	SignalDemodulator();
	~SignalDemodulator();

	/////////////////////
	// Data management //
	/////////////////////

	//Set SignalDemodulator data
	int SetIIRAlpha(double iir_alpha);
	int SetNoiseFloorDB(double noise_floor_db);
	int SetSignalThresholdDB(double signal_threshold_db);

	int SetPortAudioData(PortAudioData* port_audio_data);

	int SetLogFileManager(LogFileManager* logfile_manager);

	//Get SignalDemodulator data

	//////////////////////
	// COMPSK functions //
	//////////////////////

	int Initialize();
	int RunDemodulation();

	int CalculateIIR();
	int CalculateCrossCorrelation();
	int CalculateSynchronizationCrossCorrelation();

	/////////////////////////
	// Debugging functions //
	/////////////////////////

	int Print();

private:
	////////////////////////////
	// Internal use functions //
	////////////////////////////

	int CreateSynchronizationSignal();

	int CreateFIRCorrelationVector();

	//Demodulation functions
	int FindFirstSymbol(int& found_symbol, size_t& first_symbol_index);

	int FindFirstSymbol_IIR(int& found_symbol, size_t& first_symbol_index);
	int FindFirstSymbol_CrossCorrelation(int& found_symbol, size_t& first_symbol_index);

	int DemodulateSymbols(size_t first_symbol_index);
	int AppendDemodulationBit(int bit_value);

	int GatherSignalData();

	///////////////////////
	// Internal use data //
	///////////////////////

	//Signal data
	int symbol_period;
	int symbol_length;

	double noise_floor_db;
	double signal_threshold_db;

	vector< complex<double> > signal_vector;

	double iir_alpha;
	vector< complex<double> > iir_vector;

	double fir_sigma;
	double fir_cutoff_amplitude;
	vector< complex<double> > fir_correlation_vector;
	vector< complex<double> > fir_result_vector;

	int demodulation_char_bit_index;
	vector<char> demodulation_vector;

	string synch_signal_message;
	vector< complex<double> > synch_signal_vector;

	//Application data
	PortAudioData* port_audio_data;
	LogFileManager* logfile_manager;
};

#endif
