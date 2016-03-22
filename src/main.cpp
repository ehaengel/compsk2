#include <stdlib.h>
#include <stdio.h>

// COMPSK library
#include "communication_server.h"

int main(int argc, char** argv)
{
	LogFileManager* logfile_manager = new LogFileManager;
	logfile_manager->VerboseModeOn();

	CommunicationServer* communication_server = new CommunicationServer;
	communication_server->SetLogFileManager(logfile_manager);

	if(argc > 1)
	{
		int port_number = -1;
		sscanf(argv[1], "%d", &port_number);

		communication_server->SetPortNumber(port_number);
	}

	if(communication_server->Initialize() == false)
	{
		logfile_manager->WriteErrorMessage("main", "Could not initialize communication server");
		return 1;
	}

	if(communication_server->RunCommunicationServer() == false)
	{
		logfile_manager->WriteErrorMessage("main", "Could not run communication server");
		return 1;
	}



	/*AudioServer* audio_server = new AudioServer;
	audio_server->SetLogFileManager(logfile_manager);

	if(audio_server->Initialize() == false)
	{
		logfile_manager->WriteErrorMessage("main", "Could not initialize audio server");
		return 1;
	}

	SignalModulator* signal_modulator = new SignalModulator;
	signal_modulator->SetPortAudioData(audio_server->GetPortAudioData());
	signal_modulator->SetLogFileManager(logfile_manager);

	if(signal_modulator->Initialize() == false)
	{
		logfile_manager->WriteErrorMessage("main", "Could not initialize signal modulator");
		return 1;
	}

	signal_modulator->ModulateStringData("hello world!");
	//audio_server->GetPortAudioData()->audio_output_fifo->SaveToFile("data/cyclic_fifo.dat");

	PortAudioData* pa_data = audio_server->GetPortAudioData();
	while(pa_data->play_audio != 2)
	{
		printf("Enter a number:\n");
		scanf("%d", &pa_data->play_audio);
	}

	delete audio_server;
	delete signal_modulator;*/
	return 0;
}
