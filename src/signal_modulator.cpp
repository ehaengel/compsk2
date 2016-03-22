#include "signal_modulator.h"

SignalModulator::SignalModulator()
{
	//Modulation data
	symbol_period = 0;
	symbol_length = 0;

	synch_signal_message = "ACK";
	synch_signal_vector.clear();

	//Application data
	port_audio_data = NULL;
	logfile_manager = NULL;
}

SignalModulator::~SignalModulator()
{
	//Do nothing
}

/////////////////////
// Data management //
/////////////////////

//Set SignalModulator data
int SignalModulator::SetPortAudioData(PortAudioData* port_audio_data)
{
	this->port_audio_data = port_audio_data;

	return true;
}

int SignalModulator::SetLogFileManager(LogFileManager* logfile_manager)
{
	this->logfile_manager = logfile_manager;

	return true;
}

//Get SignalModulator data

//////////////////////
// COMPSK functions //
//////////////////////

int SignalModulator::Initialize()
{
	if(logfile_manager == NULL)
		return false;

	//Set default modulation constants
	symbol_period = 49;
	symbol_length = 10;

	synch_signal_message = "r";
	synch_signal_vector.clear();

	//Create signal used for synchronization
	if(CreateSynchronizationSignal() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModulator", "Could not create synchronization signal");
		return false;
	}

	return true;
}

int SignalModulator::ModulateCharacterData(char c)
{
	if(port_audio_data == NULL)
		return false;

	CyclicFifo* audio_output_fifo = port_audio_data->GetAudioOutputFifo();

	double multiplier = 1.00;
	complex<double> signal_value = 0.00;
	for(int i=0; i<8; i++)
	{
		multiplier = 1.00;
		if(c & (1 << i))
			multiplier = -1.00;

		for(int j=0; j<symbol_period*symbol_length; j++)
		{
			signal_value = multiplier * sin(2*M_PI*double(j)/double(symbol_period));
			audio_output_fifo->WriteData(signal_value);
		}
	}

	return true;
}

int SignalModulator::ModulateStringData(string message)
{
	if(TransmitSynchronizationSignal() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModulator", "Could not transmit synchronization signal");
		return false;
	}

	for(int i=0; i<message.length(); i++)
	{
		if(ModulateCharacterData(message[i]) == false)
			return false;
	}

	return true;
}

int SignalModulator::TransmitSynchronizationSignal()
{
	CyclicFifo* audio_output_fifo = port_audio_data->GetAudioOutputFifo();
	for(size_t i=0; i<synch_signal_vector.size(); i++)
		audio_output_fifo->WriteData(synch_signal_vector[i]);

	return true;
}

/////////////////////////
// Debugging functions //
/////////////////////////

int SignalModulator::Print()
{

	return true;
}

////////////////////////////
// Internal use functions //
////////////////////////////

int SignalModulator::CreateSynchronizationSignal()
{
	synch_signal_vector.clear();

	double multiplier = 1.00;
	complex<double> signal_value = 0.00;
	for(size_t i=0; i<synch_signal_message.length(); i++)
	{
		char c = synch_signal_message[i];

		for(int j=0; j<8; j++)
		{
			multiplier = 1.00;
			if(c & (1 << j))
				multiplier = -1.00;

			for(int k=0; k<symbol_period*symbol_length; k++)
			{
				signal_value = multiplier * sin(2*M_PI*double(k)/double(symbol_period));
				synch_signal_vector.push_back(signal_value);
			}
		}
	}

	return true;
}
