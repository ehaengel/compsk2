#include <stdlib.h>
#include <stdio.h>

#include <complex>
#include <vector>
#include <string>
using namespace std;

// Logfile manager library
#include "logfile_manager/logfile_manager.h"

#ifndef COMPSK_CYCLIC_FIFO_H
#define COMPSK_CYCLIC_FIFO_H

#define DEFAULT_FIFO_SIZE 30*44100

class CyclicFifo
{
public:
	CyclicFifo();
	CyclicFifo(size_t fifo_size);

	~CyclicFifo();

	/////////////////////
	// Data management //
	/////////////////////

	// Set CyclicFifo data
	int WriteData(complex<double> value);

	int SetLogFileManager(LogFileManager* logfile_manager);

	// Get CyclicFifo data
	int ReadData(complex<double>& value);

	size_t GetFifoSize();
	size_t GetRemainingSpace();

	//////////////////////
	// COMPSK functions //
	//////////////////////

	int Initialize();

	int TestFifoFull();
	int TestFifoEmpty();

	int SaveToFile(string filename);
	int LoadFromFile(string filename);

	/////////////////////////
	// Debugging functions //
	/////////////////////////

	int Print();

private:
	////////////////////////////
	// Internal use functions //
	////////////////////////////

	///////////////////////
	// Internal use data //
	///////////////////////

	size_t fifo_size;
	complex<double>* fifo_data;

	size_t write_pointer;
	size_t read_pointer;

	LogFileManager* logfile_manager;
};

#endif
