#include "audio_server.h"

int audio_server_callback(const void* input_buffer, void* output_buffer, unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, void* callback_data)
{
	float* in = (float*) input_buffer;
	float* out = (float*) output_buffer;
	PortAudioData* port_audio_data = (PortAudioData*) callback_data;

	CyclicFifo* audio_input_fifo = port_audio_data->GetAudioInputFifo();
	CyclicFifo* audio_output_fifo = port_audio_data->GetAudioOutputFifo();

	complex<double> value = 0.00;
	for(unsigned long i=0; i<frames_per_buffer; i++)
	{
		if(port_audio_data->GetAudioMode() == PortAudioData::STANDBY_MODE)
		{
			*out++ = 0.00;
			*in++;

			if(CHANNEL_COUNT == 2)
			{
				*out++ = 0.00;
				*in++;
			}
		}

		else if(port_audio_data->GetAudioMode() == PortAudioData::RECORD_MODE)
		{
			*out++ = 0.00;
			audio_input_fifo->WriteData(*in++);

			if(CHANNEL_COUNT == 2)
			{
				*out++ = 0.00;
				*in++;
			}
		}

		else if(port_audio_data->GetAudioMode() == PortAudioData::PLAY_MODE)
		{
			audio_output_fifo->ReadData(value);
			*out++ = real(value);
			*in++;

			if(CHANNEL_COUNT == 2)
			{
				*out++ = real(value);
				*in++;
			}
		}

		else if(port_audio_data->GetAudioMode() == PortAudioData::PLAYBACK_MODE)
		{
			
			audio_input_fifo->ReadData(value);
			*out++ = real(value);
			*in++;

			if(CHANNEL_COUNT == 2)
			{
				*out++ = real(value);
				*in++;
			}
		}

		else if(port_audio_data->GetAudioMode() == PortAudioData::NOISE_MODE)
		{
			*out++ = (2.00 * double(rand()) / double(RAND_MAX)) - 1.00;
			*in++;

			if(CHANNEL_COUNT == 2)
			{
				*out++ = (2.00 * double(rand()) / double(RAND_MAX)) - 1.00;
				*in++;
			}
		}
	}

	return paContinue;
}

void audio_server_finished(void* callback_data)
{
	printf("audio_server_finished called\n");
}


AudioServer::AudioServer()
{
	//Port audio data
	pa_stream_parameters = NULL;
	pa_stream = NULL;

	port_audio_data = NULL;

	//Logfile manager
	logfile_manager = NULL;
}

AudioServer::~AudioServer()
{
	ClosePortAudioStream();
}

/////////////////////
// Data management //
/////////////////////

//Set AudioServer data
int AudioServer::SetLogFileManager(LogFileManager* logfile_manager)
{
	this->logfile_manager = logfile_manager;

	return true;
}

//Get AudioServer data
PortAudioData* AudioServer::GetPortAudioData()
{
	return port_audio_data;
}

//////////////////////
// COMPSK functions //
//////////////////////

int AudioServer::Initialize()
{
	if(logfile_manager == NULL)
		return false;

	if(InitializePortAudioData() == false)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not initialize port audio data");
		return false;
	}

	if(InitializePortAudio() == false)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not initialize port audio");
		return false;
	}

	return true;
}

/////////////////////////
// Debugging functions //
/////////////////////////

int AudioServer::Print()
{

	return true;
}

////////////////////////////
// Internal use functions //
////////////////////////////

int AudioServer::InitializePortAudioData()
{
	port_audio_data = new PortAudioData;
	port_audio_data->SetLogFileManager(logfile_manager);
	if(port_audio_data->Initialize() == false)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not initialize port audio data");
		return false;
	}

	return true;
}

int AudioServer::InitializePortAudio()
{
	PaError pa_error = Pa_Initialize();
	if(pa_error != paNoError)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not initialize pulse audio");
		logfile_manager->WriteErrorMessage("AudioServer", Pa_GetErrorText(pa_error));
		return false;
	}

	pa_stream_parameters = new PaStreamParameters;
	pa_stream_parameters->device = Pa_GetDefaultOutputDevice();
	if(pa_stream_parameters->device == paNoDevice)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not set device to default device\n");
		logfile_manager->WriteErrorMessage("AudioServer", Pa_GetErrorText(pa_error));

		return false;
	}

	pa_stream_parameters->channelCount = CHANNEL_COUNT;
	pa_stream_parameters->sampleFormat = paFloat32;
	pa_stream_parameters->suggestedLatency = Pa_GetDeviceInfo(pa_stream_parameters->device)->defaultLowOutputLatency;
	pa_stream_parameters->hostApiSpecificStreamInfo = NULL;

	pa_error = Pa_OpenStream(&pa_stream, pa_stream_parameters, pa_stream_parameters, 44100, 4500, paClipOff, audio_server_callback, (void*) port_audio_data);
	if(pa_error != paNoError)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not open stream");
		logfile_manager->WriteErrorMessage("AudioServer", Pa_GetErrorText(pa_error));

		return false;
	}

	pa_error = Pa_SetStreamFinishedCallback(pa_stream, &audio_server_finished);
	if(pa_error != paNoError)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not set stream finished callback");
		logfile_manager->WriteErrorMessage("AudioServer", Pa_GetErrorText(pa_error));

		return false;
	}

	pa_error = Pa_StartStream(pa_stream);
	if(pa_error != paNoError)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not start stream");
		logfile_manager->WriteErrorMessage("AudioServer", Pa_GetErrorText(pa_error));

		return false;
	}

	return true;
}

int AudioServer::ClosePortAudioStream()
{
	PaError pa_error = Pa_StopStream(pa_stream);
	if(pa_error != paNoError)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not stop stream");
		logfile_manager->WriteErrorMessage("AudioServer", Pa_GetErrorText(pa_error));

		return false;
	}

	pa_error = Pa_CloseStream(pa_stream);
	if(pa_error != paNoError)
	{
		logfile_manager->WriteErrorMessage("AudioServer", "Could not close stream");
		logfile_manager->WriteErrorMessage("AudioServer", Pa_GetErrorText(pa_error));

		return false;
	}

	Pa_Terminate();
	return true;
}
