#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <complex>
#include <vector>
using namespace std;

// COMPSK Library
#include "audio_server.h"

#include "signal_modem.h"

#ifndef COMPSK_COMMUNICATION_SERVER_H
#define COMPSK_COMMUNICATION_SERVER_H

class CommunicationServer
{
public:
	CommunicationServer();
	~CommunicationServer();

	/////////////////////
	// Data management //
	/////////////////////

	//Set CommunicationServer data
	int SetPortNumber(int port_number);

	int SetLogFileManager(LogFileManager* logfile_manager);

	//Get CommunicationServer data

	//////////////////////
	// COMPSK functions //
	//////////////////////

	int Initialize();
	int RunCommunicationServer();

	/////////////////////////
	// Debugging functions //
	/////////////////////////

	int Print();

private:
	////////////////////////////
	// Internal use functions //
	////////////////////////////

	//Initialization
	int InitializeSocket();
	int InitializeAudioServer();
	int InitializeSignalProcessing();

	//Server communication management
	int WaitForConnection();

	int ParseMessages(int& found_message);
	int WriteHelp();

	//Server management
	int CloseConnection();
	int ShutdownServer();

	///////////////////////
	// Internal use data //
	///////////////////////

	// Socket data
	int sockfd;
	int port_number;

	int client_sockfd;
	FILE* client_handle;
	string client_ip_address;

	sockaddr_in* server_addr;
	sockaddr_in* client_addr;

	int communication_mode;
	enum
	{
		DEFAULT_MODE = 0,
		DATA_MODE = 1,
		SHUTDOWN_MODE = 2
	};

	//Signal processing data
	SignalModem* signal_modem;

	//Application data
	AudioServer* audio_server;
	LogFileManager* logfile_manager;
};

#endif


