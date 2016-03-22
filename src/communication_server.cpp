#include "communication_server.h"

CommunicationServer::CommunicationServer()
{
	//Socket data
	sockfd = -1;
	port_number = 10000;

	client_sockfd = -1;
	client_handle = NULL;
	client_ip_address = "";

	server_addr = NULL;
	client_addr = NULL;

	communication_mode = CommunicationServer::DEFAULT_MODE;

	//Signal processing data
	signal_modem = NULL;

	//Application data
	logfile_manager = NULL;
}

CommunicationServer::~CommunicationServer()
{
	if(signal_modem != NULL)
	{
		delete signal_modem;
		signal_modem = NULL;
	}
	if(audio_server != NULL)
	{
		delete audio_server;
		audio_server = NULL;
	}
}

/////////////////////
// Data management //
/////////////////////

//Set CommunicationServer data
int CommunicationServer::SetPortNumber(int port_number)
{
	this->port_number = port_number;

	return true;
}

int CommunicationServer::SetLogFileManager(LogFileManager* logfile_manager)
{
	this->logfile_manager = logfile_manager;

	return true;
}

//Get CommunicationServer data

//////////////////////
// COMPSK functions //
//////////////////////

int CommunicationServer::Initialize()
{
	if(logfile_manager == NULL)
		return false;

	if(InitializeSocket() == false)
	{
		logfile_manager->WriteErrorMessage("CommunicationServer", "Could not initialize socket");
		return false;
	}

	if(InitializeAudioServer() == false)
	{
		logfile_manager->WriteErrorMessage("CommunicationServer", "Could not initialize audio server");
		return false;
	}

	if(InitializeSignalProcessing() == false)
	{
		logfile_manager->WriteErrorMessage("CommunicationServer", "Could not initialize signal processing");
		return false;
	}

	return true;
}

int CommunicationServer::RunCommunicationServer()
{
	while(communication_mode != CommunicationServer::SHUTDOWN_MODE)
	{
		if(client_sockfd == -1)
		{
			if(WaitForConnection() == false)
			{
				logfile_manager->WriteErrorMessage("CommunicationServer", "Could not wait for connection");
				return false;
			}
		}

		else
		{
			int found_message = false;
			if(ParseMessages(found_message) == false)
			{
				logfile_manager->WriteErrorMessage("CommunicationServer", "Could not parse messages");
				return false;
			}
		}

		if(signal_modem->RunModem() == false)
		{
			logfile_manager->WriteErrorMessage("CommunicationServer", "Could not run signal modem");
			return false;
		}
	}

	if(ShutdownServer() == false)
	{
		logfile_manager->WriteErrorMessage("CommunicationServer", "Could not shutdown server");
		return false;
	}

	return true;
}

/////////////////////////
// Debugging functions //
/////////////////////////

int CommunicationServer::Print()
{

	return true;
}

////////////////////////////
// Internal use functions //
////////////////////////////

//Initialization
int CommunicationServer::InitializeSocket()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		logfile_manager->WriteErrorMessage("CommunicationServer", "Could not initialize socket");
		return false;
	}

	client_addr = new sockaddr_in;
	server_addr = new sockaddr_in;
	bzero((char*) server_addr, sizeof(*server_addr));
	bzero((char*) client_addr, sizeof(*client_addr));

	server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = INADDR_ANY;
	server_addr->sin_port = htons(port_number);

	if(bind(sockfd, (struct sockaddr*) server_addr, sizeof(*server_addr)) < 0)
	{
		logfile_manager->WriteErrorMessage("CommunicationServer", "Could not bind socket");
		return false;
	}
	listen(sockfd,5);

	return true;
}

int CommunicationServer::InitializeAudioServer()
{
	audio_server = new AudioServer;
	audio_server->SetLogFileManager(logfile_manager);

	if(audio_server->Initialize() == false)
	{
		logfile_manager->WriteErrorMessage("CommunicationServer", "Could not initialize audio server");
		return false;
	}

	return true;
}

int CommunicationServer::InitializeSignalProcessing()
{
	signal_modem = new SignalModem;
	signal_modem->SetPortAudioData(audio_server->GetPortAudioData());
	signal_modem->SetLogFileManager(logfile_manager);

	if(signal_modem->Initialize() == false)
	{
		logfile_manager->WriteErrorMessage("CommunicationSErver", "Could not initialize signal modem");
		return false;
	}

	return true;
}

//Server communication management
int CommunicationServer::WaitForConnection()
{
	//Check if accept will block
	fd_set read_set;
	timeval timeout;

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	FD_ZERO(&read_set);
	FD_SET(sockfd, &read_set);

	int result = select(sockfd+1, &read_set, NULL, NULL, &timeout);
	if(FD_ISSET(sockfd, &read_set) == false)
	{
		logfile_manager->WriteNormalMessage("CommunicationServer", "No connection available");
		return true;
	}

	//Accept connection
	socklen_t client_length = sizeof(*client_addr);

	client_sockfd = accept(sockfd, (struct sockaddr*) client_addr, &client_length);
	if(client_sockfd < 0)
	{
		logfile_manager->WriteErrorMessage("CommunicationServer", "Could not accept client");
		return false;
	}
	client_handle = fdopen(client_sockfd, "r+");

	char buffer[1000];
	inet_ntop(AF_INET, &client_addr->sin_addr, buffer, INET_ADDRSTRLEN);
	client_ip_address = buffer;
	logfile_manager->WriteNormalMessage("CommunicationServer", "Connected to " + client_ip_address);

	return true;
}

int CommunicationServer::ParseMessages(int& found_message)
{
	char buffer[1000];

	fd_set read_set;
	timeval timeout;

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	FD_ZERO(&read_set);
	FD_SET(client_sockfd, &read_set);

	int result = select(client_sockfd+1, &read_set, NULL, NULL, &timeout);

	if(FD_ISSET(client_sockfd, &read_set))
	{
		found_message = true;

		if(fgets(buffer, 1000, client_handle) == NULL)
		{
			if(CloseConnection() == false)
			{
				logfile_manager->WriteErrorMessage("CommunicationServer", "Could not close connection");
				return false;
			}

			return true;
		}

		else if(strncmp(buffer, "help\n", 5) == 0)
		{
			WriteHelp();
		}

		else if(strncmp(buffer, "standby\n", 8) == 0)
		{
			PortAudioData* port_audio_data = audio_server->GetPortAudioData();
			port_audio_data->SetAudioMode(PortAudioData::STANDBY_MODE);
		}

		else if(strncmp(buffer, "record\n", 7) == 0)
		{
			PortAudioData* port_audio_data = audio_server->GetPortAudioData();
			port_audio_data->SetAudioMode(PortAudioData::RECORD_MODE);
		}

		else if(strncmp(buffer, "playback\n", 9) == 0)
		{
			PortAudioData* port_audio_data = audio_server->GetPortAudioData();
			port_audio_data->SetAudioMode(PortAudioData::PLAYBACK_MODE);
		}

		else if(strncmp(buffer, "play\n", 9) == 0)
		{
			PortAudioData* port_audio_data = audio_server->GetPortAudioData();
			port_audio_data->SetAudioMode(PortAudioData::PLAY_MODE);
		}

		else if(strncmp(buffer, "noise\n", 6) == 0)
		{
			PortAudioData* port_audio_data = audio_server->GetPortAudioData();
			port_audio_data->SetAudioMode(PortAudioData::NOISE_MODE);
		}

		else if(strncmp(buffer, "transmit", 8) == 0)
		{
			if(strlen(buffer) > 9)
				signal_modem->TransmitMessage(buffer + 9);

			else
			{
				FILE* handle = fopen("data/message", "r");

				if(handle != NULL)
				{
					char message[1000];
					fgets(message, 1000, handle);
					signal_modem->TransmitMessage(message);
					fclose(handle);
				}
			}
		}

		else if(strncmp(buffer, "tone", 4) == 0)
		{
			size_t tone_length = 0;
			sscanf(buffer, "tone %lu", &tone_length);
			BitList tone_bit_list(tone_length, 0x01);
			printf("tone_bit_list.size() = %lu\n", tone_bit_list.size());
			signal_modem->TransmitMessage(tone_bit_list);
		}

		else if(strncmp(buffer, "demodulate on", 13) == 0)
			signal_modem->SetDemodulateOn(true);

		else if(strncmp(buffer, "demodulate off", 14) == 0)
			signal_modem->SetDemodulateOn(false);

		else if(strncmp(buffer, "load", 4) == 0)
		{
			char filename[1000];
			sscanf(buffer, "load %s", filename);

			if(strstr(filename, "\n") != NULL)
				*strstr(filename, "\n") = '\0';

			printf("filename = %s\n", filename);

			FILE* data_handle = fopen(filename, "r");
			if(data_handle == NULL)
				logfile_manager->WriteNormalMessage("CommunicationServer", "Could not open file.");

			else
			{
				double rbuf, ibuf;

				PortAudioData* port_audio_data = audio_server->GetPortAudioData();
				CyclicFifo* audio_input_fifo = port_audio_data->GetAudioInputFifo();

				fgets(buffer, 1000, data_handle);
				while(!feof(data_handle))
				{
					sscanf(buffer, "%lf %lf", &rbuf, &ibuf);
					audio_input_fifo->WriteData(complex<double>(rbuf, ibuf));

					fgets(buffer, 1000, data_handle);
				}
				logfile_manager->WriteNormalMessage("CommunicationServer", "Done loading file.");

				fclose(data_handle);
			}
		}

		else if(strncmp(buffer, "dump tx\n", 8) == 0)
		{
			PortAudioData* port_audio_data = audio_server->GetPortAudioData();
			CyclicFifo* audio_output_fifo = port_audio_data->GetAudioOutputFifo();

			audio_output_fifo->SaveToFile("data/audio_output_fifo.dat");
		}

		else if(strncmp(buffer, "dump rx\n", 8) == 0)
		{
			PortAudioData* port_audio_data = audio_server->GetPortAudioData();
			CyclicFifo* audio_input_fifo = port_audio_data->GetAudioInputFifo();

			audio_input_fifo->SaveToFile("data/audio_input_fifo.dat");
		}

		else if(strncmp(buffer, "data\n", 5) == 0)
			signal_modem->WriteDataUsage(client_handle);

		else if(strncmp(buffer, "synch\n", 6) == 0)
			signal_modem->SaveSynchronizationSignal();

		else if(strncmp(buffer, "close\n", 6) == 0)
			CloseConnection();

		else if(strncmp(buffer, "quit", 4) == 0)
			communication_mode = CommunicationServer::SHUTDOWN_MODE;
	}

	else
	{
		found_message = false;
		logfile_manager->WriteNormalMessage("CommunicationServer", "No message");
	}

	return true;
}

int CommunicationServer::WriteHelp()
{
	FILE* handle = fopen("data/help_message", "r");
	if(handle == NULL)
	{
		logfile_manager->WriteErrorMessage("CommunicationServer", "Could not open helpfile");
		return false;
	}

	char buffer[1000];
	fgets(buffer, 1000, handle);

	while(!feof(handle))
	{
		fprintf(client_handle, "%s", buffer);
		fgets(buffer, 1000, handle);
	}
	fflush(client_handle);

	return true;
}

//Server management
int CommunicationServer::CloseConnection()
{
	close(client_sockfd);
	client_sockfd = -1;
	client_handle = NULL;
	client_ip_address = "";

	communication_mode = CommunicationServer::DEFAULT_MODE;

	return true;
}

int CommunicationServer::ShutdownServer()
{
	if(client_sockfd != -1)
		close(client_sockfd);

	close(sockfd);
	return true;
}
