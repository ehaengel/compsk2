#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <complex.h>

#include <complex>
#include <vector>
using namespace std;

// Logfile manager library
#include "logfile_manager/logfile_manager.h"

// SimpleXML2 library
#include "simplexml2/xml_document.h"

// COMPSK Library
#include "cyclic_fifo.h"
#include "audio_server.h"

#ifndef COMPSK_SIGNAL_MODEM_H
#define COMPSK_SIGNAL_MODEM_H

typedef complex<double> ConstellationPoint;
typedef vector< ConstellationPoint > ConstellationPointList;

typedef vector< complex<double> > SignalVector;

typedef vector<char> BitList;

class SignalModem
{
public:
	SignalModem();
	~SignalModem();

	/////////////////////
	// Data management //
	/////////////////////

	//Set SignalModem data
	int SetSymbolPeriod(int symbol_period);
	int SetSymbolLength(int symbol_length);

	int SetDemodulateOn(int demodulate_on);

	int SetSynchronizationMessage(string synchronization_message);

	int SetFIRSigma(double fir_sigma);
	int SetFIRCutoffAmplitude(double fir_cutoff_amplitude);

	int SetPortAudioData(PortAudioData* port_audio_data);
	int SetLogFileManager(LogFileManager* logfile_manager);

	//////////////////////
	// COMPSK functions //
	//////////////////////

	int Initialize();
	int LoadConfigurationFile();

	int RunModem();

	int TransmitMessage(BitList transmission_message);
	int TransmitMessage(string transmission_message);

	//Diagnostic data
	int WriteDataUsage(FILE* client_handle);

	/////////////////////////
	// Debugging functions //
	/////////////////////////

	int Print();

	int SaveSynchronizationSignal();

private:
	////////////////////////////
	// Internal use functions //
	////////////////////////////

	//Initialization functions
	int CreateSynchronizationSignal();

	//Receiver functions
	int DemodulateSignal();
	//int DemodulateSignal(size_t message_start_index, size_t& message_stop_index);

	int CollectSignalData();

	int CalculateDiagnosticData();
	int CleanupSignalVectors();

	//Transmission functions
	int InitializeTransmissionSignal();
	int AppendToTransmissionSignal(string str);

	int SendTransmissionSignal();

	//General purpose functions
	int SynthesizeByte(SignalVector& signal_vector, char c);
	int SynthesizeBit(SignalVector& signal_vector, int b);

	int SynthesizeConstellationPoints(SignalVector& signal_vector, char c);

	//int UnsynthesizeConstellationPoints(string& message, SignalVector signal_vector);
	int UnsynthesizeConstellationPoints(BitList& bit_list, ConstellationPointList constellation_point_list);

	int ResetModemData();
	int LoadModemConfiguration(XML_Content* modem_configuration_tag);

	///////////////////////
	// Internal use data //
	///////////////////////

	//Modulation data
	int symbol_period;
	int symbol_length;

	int demodulate_on;

	//Synchronization data
	BitList synchronization_bit_list;
	ConstellationPointList synchronization_constellation_point_list;

	string synchronization_message;
	SignalVector synchronization_transmit_signal;
	SignalVector synchronization_receive_signal;

	//Receiver data
	SignalVector received_signal;
	string received_message;

	ConstellationPointList constellation_point_fifo;

	double phase_index;

	//Transmitter data
	string transmission_message;
	SignalVector transmission_signal;

	//Application data
	PortAudioData* port_audio_data;
	LogFileManager* logfile_manager;
};

#endif


