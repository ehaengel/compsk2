#include "cyclic_fifo.h"

CyclicFifo::CyclicFifo()
{
	fifo_size = DEFAULT_FIFO_SIZE;
	fifo_data = NULL;

	write_pointer = 0;
	read_pointer = 0;

	logfile_manager = NULL;
}

CyclicFifo::CyclicFifo(size_t fifo_size)
{
	this->fifo_size = fifo_size;
	fifo_data = NULL;

	write_pointer = 0;
	read_pointer = 0;

	logfile_manager = NULL;
}

CyclicFifo::~CyclicFifo()
{
	if(fifo_data != NULL)
	{
		delete fifo_data;
		fifo_data = NULL;
	}
}

/////////////////////
// Data management //
/////////////////////

// Set CyclicFifo data
int CyclicFifo::WriteData(complex<double> value)
{
	//Fifo is full or not initialized
	if(fifo_data == NULL || TestFifoFull())
		return false;

	fifo_data[write_pointer] = value;
	write_pointer = (write_pointer + 1) % fifo_size;

	return true;
}

int CyclicFifo::SetLogFileManager(LogFileManager* logfile_manager)
{
	this->logfile_manager = logfile_manager;

	return true;
}

// Get CyclicFifo data
int CyclicFifo::ReadData(complex<double>& value)
{
	//Fifo is empty or not initialized
	if(fifo_data == NULL || TestFifoEmpty())
		return false;

	value = fifo_data[read_pointer];
	read_pointer = (read_pointer+1) % fifo_size;

	return true;
}

size_t CyclicFifo::GetFifoSize()
{
	return fifo_size;
}

size_t CyclicFifo::GetRemainingSpace()
{
	return (((read_pointer + fifo_size) - write_pointer) % fifo_size);
}

//////////////////////
// COMPSK functions //
//////////////////////

int CyclicFifo::Initialize()
{
	if(logfile_manager == NULL)
		return false;

	fifo_data = new complex<double>[fifo_size];
	for(size_t i=0; i<fifo_size; i++)
		fifo_data[i] = 0.00;

	return true;
}

int CyclicFifo::TestFifoFull()
{
	if((write_pointer+1) % fifo_size == read_pointer)
		return true;

	return false;
}

int CyclicFifo::TestFifoEmpty()
{
	if(read_pointer == write_pointer)
		return true;

	return false;
}

int CyclicFifo::SaveToFile(string filename)
{
	if(fifo_data == NULL)
		return false;

	FILE* handle = fopen(filename.c_str(), "w");
	if(handle == NULL)
		return false;

	complex<double> value = 0.00;
	while(ReadData(value))
		fprintf(handle, "%.8lf %.8lf\n", real(value), imag(value));

	fclose(handle);
	return true;
}

int CyclicFifo::LoadFromFile(string filename)
{
	if(fifo_data == NULL)
		return false;

	FILE* handle = fopen(filename.c_str(), "r");
	if(handle == NULL)
	{
		printf("filename = %s\n", filename.c_str());
		logfile_manager->WriteErrorMessage("CyclicFifo", "Unable to open file for reading");
		return false;
	}

	char buffer[1000];
	double value_r = 0.00;
	double value_i = 0.00;

	int count = 0;
	fgets(buffer, 1000, handle);
	while(!feof(handle))
	{
		sscanf(buffer, "%lf %lf", &value_r, &value_i);

		if(WriteData(complex<double>(value_r, value_i)) == false)
			break;

		fgets(buffer, 1000, handle);
		count++;
	}

	fclose(handle);
	return true;
}

/////////////////////////
// Debugging functions //
/////////////////////////

int CyclicFifo::Print()
{
	printf("write_pointer = %lu\n", write_pointer);
	printf("read_pointer = %lu\n", read_pointer);
	printf("fifo_size = %lu\n", fifo_size);

	return true;
}
