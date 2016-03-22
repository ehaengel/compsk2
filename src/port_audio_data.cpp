#include "port_audio_data.h"

PortAudioData::PortAudioData()
{
	//Audio data
	audio_input_fifo = NULL;
	audio_output_fifo = NULL;

	audio_mode = PortAudioData::STANDBY_MODE;

	//Application data
	logfile_manager = NULL;
}

PortAudioData::~PortAudioData()
{
	if(audio_input_fifo != NULL)
	{
		delete audio_input_fifo;
		audio_input_fifo = NULL;
	}

	if(audio_output_fifo != NULL)
	{
		delete audio_output_fifo;
		audio_output_fifo = NULL;
	}
}

/////////////////////
// Data management //
/////////////////////

//Set PortAudio data
int PortAudioData::SetAudioMode(int audio_mode)
{
	this->audio_mode = audio_mode;

	return true;
}

int PortAudioData::SetLogFileManager(LogFileManager* logfile_manager)
{
	this->logfile_manager = logfile_manager;

	return true;
}

//Get PortAudio data
CyclicFifo* PortAudioData::GetAudioInputFifo()
{
	return audio_input_fifo;
}

CyclicFifo* PortAudioData::GetAudioOutputFifo()
{
	return audio_output_fifo;
}

int PortAudioData::GetAudioMode()
{
	return audio_mode;
}

//////////////////////
// COMPSK functions //
//////////////////////

int PortAudioData::Initialize()
{
	if(logfile_manager == NULL)
		return false;

	audio_input_fifo = new CyclicFifo;
	audio_input_fifo->SetLogFileManager(logfile_manager);
	if(audio_input_fifo->Initialize() == false)
		return false;

	audio_output_fifo = new CyclicFifo;
	audio_output_fifo->SetLogFileManager(logfile_manager);
	if(audio_output_fifo->Initialize() == false)
		return false;

	return true;
}

/////////////////////////
// Debugging functions //
/////////////////////////

int PortAudioData::Print()
{

	return true;
}
