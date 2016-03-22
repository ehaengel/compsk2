#include "signal_demodulator.h"

SignalDemodulator::SignalDemodulator()
{
	//Signal data
	symbol_period = 0;
	symbol_length = 0;

	noise_floor_db = 30;
	signal_threshold_db = 10;

	signal_vector.clear();

	synch_signal_message = "ACK";
	synch_signal_vector.clear();

	fir_sigma = 100;
	fir_cutoff_amplitude = 0.1;
	fir_correlation_vector.clear();
	fir_result_vector.clear();

	iir_alpha = 0.0001;
	iir_vector.clear();

	demodulation_char_bit_index = 0;
	demodulation_vector.clear();

	//Application data
	port_audio_data;
	logfile_manager = NULL;
}

SignalDemodulator::~SignalDemodulator()
{
	//Do nothing
}

/////////////////////
// Data management //
/////////////////////

//Set SignalDemodulator data
int SignalDemodulator::SetIIRAlpha(double iir_alpha)
{
	this->iir_alpha = iir_alpha;

	return true;
}

int SignalDemodulator::SetNoiseFloorDB(double noise_floor_db)
{
	this->noise_floor_db = noise_floor_db;

	return true;
}

int SignalDemodulator::SetSignalThresholdDB(double signal_threshold_db)
{
	this->signal_threshold_db = signal_threshold_db;

	return true;
}

int SignalDemodulator::SetPortAudioData(PortAudioData* port_audio_data)
{
	this->port_audio_data = port_audio_data;

	return true;
}

int SignalDemodulator::SetLogFileManager(LogFileManager* logfile_manager)
{
	this->logfile_manager = logfile_manager;
}

//////////////////////
// COMPSK functions //
//////////////////////

int SignalDemodulator::Initialize()
{
	if(logfile_manager == NULL)
		return false;

	//Set default demodulation constants
	symbol_period = 49;
	symbol_length = 10;

	noise_floor_db = -80;
	signal_threshold_db = 20;

	synch_signal_message = "r";
	synch_signal_vector.clear();

	//Create signal used for synchronization
	if(CreateSynchronizationSignal() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModulator", "Could not create synchronization signal");
		return false;
	}

	if(CreateFIRCorrelationVector() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModulator", "Could not create fir correlation vector");
		return false;
	}

	return true;
}

int SignalDemodulator::RunDemodulation()
{
	//Gather signal data
	if(GatherSignalData() == false)
	{
		logfile_manager->WriteErrorMessage("SignalDemodulator", "Could not gather signal data");
		return false;
	}

	//Find occurance of first symbol
	int found_symbol = false;
	size_t first_symbol_index = 0;
	if(FindFirstSymbol(found_symbol, first_symbol_index) == false)
	{
		logfile_manager->WriteErrorMessage("SignalDemodulator", "Could not find first symbol");
		return false;
	}
	if(found_symbol == false)
		return true;

	//Decode detected symbols
	if(DemodulateSymbols(first_symbol_index) == false)
	{
		logfile_manager->WriteErrorMessage("SymbolDecoder", "Could not decode symbols");
		return false;
	}


	return true;
}

int SignalDemodulator::CalculateIIR()
{
	if(GatherSignalData() == false)
	{
		logfile_manager->WriteErrorMessage("SignalDemodulator", "Could not gather signal data");
		return false;
	}

	printf("signal_vector.size() = %lu\n", signal_vector.size());
	if(signal_vector.size() == 0)
		return true;

	printf("iir_alpha = %.8lf\n", iir_alpha);

	iir_vector.clear();
	iir_vector.resize(signal_vector.size(), 0.00);

	double omega = 2*M_PI/double(symbol_period);
	for(size_t i=1; i<signal_vector.size(); i++)
	{
		double phi = omega*double(i % symbol_period);
		iir_vector[i] = (1.00-iir_alpha)*iir_vector[i-1] + iir_alpha*signal_vector[i]*complex<double>(cos(phi), sin(phi));
	}

	FILE* handle = fopen("data/iir.dat", "w");
	for(size_t i=0; i<iir_vector.size(); i++)
		fprintf(handle, "%.8lf %.8lf %.8lf %.8lf\n", real(iir_vector[i]), imag(iir_vector[i]), real(signal_vector[i]), imag(signal_vector[i]));

	fclose(handle);

	signal_vector.clear();

	return true;
}

int SignalDemodulator::CalculateCrossCorrelation()
{
	if(GatherSignalData() == false)
	{
		logfile_manager->WriteErrorMessage("SignalDemodulator", "Could not gather signal data");
		return false;
	}

	vector< complex<double> > xcorr_vector;
	xcorr_vector.resize(signal_vector.size(), 0.00);

	printf("signal_vector.size() = %lu\n", signal_vector.size());
	printf("xcorr_vector.size() = %lu\n", xcorr_vector.size());
	if(signal_vector.size() == 0)
		return true;

	double omega = 2*M_PI/double(symbol_period);
	for(size_t i=0; i+(symbol_period*symbol_length)<signal_vector.size(); i++)
	{
		xcorr_vector[i] = 0.00;
		for(int k=0; k<symbol_period*symbol_length; k++)
			xcorr_vector[i] = xcorr_vector[i] + signal_vector[i + k]*sin(omega*double(k % symbol_period));

	}

	FILE* handle = fopen("data/xcorr.dat", "w");
	for(size_t i=0; i<xcorr_vector.size(); i++)
		fprintf(handle, "%.8lf %.8lf\n", real(xcorr_vector[i]), imag(xcorr_vector[i]));

	fclose(handle);

	signal_vector.clear();

	return true;
}

int SignalDemodulator::CalculateSynchronizationCrossCorrelation()
{
	if(GatherSignalData() == false)
	{
		logfile_manager->WriteErrorMessage("SignalDemodulator", "Could not gather signal data");
		return false;
	}

	vector< complex<double> > xcorr_vector;
	xcorr_vector.resize(signal_vector.size(), 0.00);

	printf("signal_vector.size() = %lu\n", signal_vector.size());
	printf("xcorr_vector.size() = %lu\n", xcorr_vector.size());
	if(signal_vector.size() == 0)
		return true;

	double omega = 2*M_PI/double(symbol_period);
	for(size_t i=0; i+synch_signal_vector.size()<signal_vector.size(); i++)
	{
		xcorr_vector[i] = 0.00;
		for(size_t k=0; k<synch_signal_vector.size(); k++)
			xcorr_vector[i] = xcorr_vector[i] + signal_vector[i + k]*synch_signal_vector[k];

	}

	FILE* handle = fopen("data/xcorr_synch.dat", "w");
	for(size_t i=0; i<xcorr_vector.size(); i++)
		fprintf(handle, "%.8lf %.8lf\n", real(xcorr_vector[i]), imag(xcorr_vector[i]));

	fclose(handle);

	signal_vector.clear();

	return true;
}

/////////////////////////
// Debugging functions //
/////////////////////////

int SignalDemodulator::Print()
{

	return true;
}

////////////////////////////
// Internal use functions //
////////////////////////////

int SignalDemodulator::CreateSynchronizationSignal()
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

int SignalDemodulator::CreateFIRCorrelationVector()
{
	double omega = 2*M_PI/double(symbol_period);

	size_t total_size = (size_t) ceil(sqrt(-log(fir_cutoff_amplitude))*fir_sigma*2.0);
	fir_correlation_vector.resize(total_size, 0.00);

	FILE* handle = fopen("data/fir_correlation_vector.dat", "w");

	double center_index = double(fir_correlation_vector.size()) / 2.0;
	for(size_t i=0; i<fir_correlation_vector.size(); i++)
	{
		double dx = double(i) - double(center_index);
		fir_correlation_vector[i] = sin(omega*double(i)) * exp(-dx*dx/(fir_sigma*fir_sigma));
		fprintf(handle, "%.8lf %.8lf\n", real(fir_correlation_vector[i]), imag(fir_correlation_vector[i]));
	}
	fclose(handle);

	return true;
}

//Demodulation functions
int SignalDemodulator::FindFirstSymbol(int& found_symbol, size_t& first_symbol_index)
{
	if(FindFirstSymbol_IIR(found_symbol, first_symbol_index) == false)
	{
		logfile_manager->WriteErrorMessage("SignalDemodulator", "Could not find first symbol with IIR");
		return false;
	}
	if(found_symbol == false)
		return true;

	if(FindFirstSymbol_CrossCorrelation(found_symbol, first_symbol_index) == false)
	{
		logfile_manager->WriteErrorMessage("SignalDemodulator", "Could not find first symbol with cross correlation");
		return false;
	}

	return true;
}

int SignalDemodulator::FindFirstSymbol_IIR(int& found_symbol, size_t& first_symbol_index)
{
	iir_vector.clear();
	iir_vector.resize(signal_vector.size(), 0.00);

	int inside_noise = true;
	int signal_length = 0;
	double iir_db = 0.00;

	FILE* handle = fopen("data/find_symbol_iir.dat", "w");

	double omega = 2*M_PI / double(symbol_period);
	for(size_t i=1; i<signal_vector.size(); i++)
	{
		complex<double> beta(cos(omega*double(i)), sin(omega*double(i)));
		//iir_vector[i] = (1.00 - iir_alpha) * iir_vector[i-1] + iir_alpha * signal_vector[i] * sin(omega*double(i % symbol_period));
		iir_vector[i] = (1.00 - iir_alpha) * iir_vector[i-1] + iir_alpha * signal_vector[i] * beta;
		iir_db = 20*log10(abs(iir_vector[i]));

		if(inside_noise && iir_db > noise_floor_db + signal_threshold_db)
		{
			inside_noise = false;
			first_symbol_index = i;
		}

		else if(inside_noise == false && iir_db < noise_floor_db + signal_threshold_db)
		{
			inside_noise = true;

			if(signal_length >= symbol_length/2)
			{
				found_symbol = true;
				break;
			}

			signal_length = 0;
		}

		if(inside_noise == false)
			signal_length++;
	}

	fclose(handle);

	if(inside_noise == false)
		found_symbol = true;

	printf("(IIR) found_symbol = %d, first_symbol_index = %lu\n", found_symbol, first_symbol_index);

	return true;
}

int SignalDemodulator::FindFirstSymbol_CrossCorrelation(int& found_symbol, size_t& first_symbol_index)
{
	double cross_cor_mag_max = 0.00;
	size_t cross_cor_mag_max_index = first_symbol_index;

	size_t start_index = 0;
	if(first_symbol_index > symbol_period*symbol_length)
		start_index = first_symbol_index - symbol_period*symbol_length;

	FILE* handle = fopen("data/xcor.dat", "w");

	for(size_t i=start_index; i<first_symbol_index && i<signal_vector.size(); i++)
	{
		complex<double> cross_cor = 0.00;

		double omega = 2*M_PI / double(symbol_period);
		/*for(int j=0; j<symbol_length; j++)
		{
			complex<double> beta(cos(omega*double(j)), sin(omega*double(j)));
			cross_cor = cross_cor + signal_vector[i+j] * beta;
			//cross_cor = cross_cor + signal_vector[i+j] * sin(omega*double(j));
		}*/
		for(int k=0; k<synch_signal_vector.size(); k++)
			cross_cor = cross_cor + signal_vector[i+k] * synch_signal_vector[k];

		fprintf(handle, "%.8lf %.8lf\n", real(cross_cor), imag(cross_cor));

		if(20*log10(abs(cross_cor)) > 20*log10(cross_cor_mag_max))
		{
			cross_cor_mag_max = abs(cross_cor);
			cross_cor_mag_max_index = i;
		}
	}
	fclose(handle);

	first_symbol_index = cross_cor_mag_max_index;
	printf("(CrossCorrelation) found_symbol = %d, first_symbol_index = %lu\n", found_symbol, first_symbol_index);


	return true;
}

int SignalDemodulator::DemodulateSymbols(size_t first_symbol_index)
{
	demodulation_vector.clear();

	size_t index = first_symbol_index;

	double omega = 2*M_PI / double(symbol_period);
	while(index < signal_vector.size())
	{
		complex<double> cross_cor = 0.00;

		for(int j=0; j<symbol_length*symbol_period; j++)
		{
			//complex<double> beta(cos(omega*double(j)), sin(omega*double(j)));
			//cross_cor = cross_cor + signal_vector[index+j] * beta / double(symbol_length);
			cross_cor = cross_cor + signal_vector[index+j] * sin(omega*double(j)) / double(symbol_length);
		}

		double cross_cor_mag_db = 20*log10(abs(cross_cor));

		if(cross_cor_mag_db < noise_floor_db + signal_threshold_db)
		{
			signal_vector.erase(signal_vector.begin(), signal_vector.begin() + index + symbol_length);
			break;
		}

		else
		{
			if(real(cross_cor) < 0.00)
				AppendDemodulationBit(0x00);

			else
				AppendDemodulationBit(0x01);
		}

		//printf("index = %lu, %s, %lf, %lf\n", index, complex_to_str(cross_cor).c_str(), fabs(arg(cross_cor) - M_PI), fabs(arg(cross_cor)));

		index = index + symbol_length*symbol_period;
	}
	if(index >= signal_vector.size())
		signal_vector.clear();

	printf("\n");

	for(size_t i=0; i<demodulation_vector.size(); i++)
	{
		for(int j=7; j>=0; j--)
		{
			if(demodulation_vector[i] & (1 << j))
				printf("1");

			else
				printf("0");
		}

		printf(" ");
	}

	printf("\n");

	for(size_t i=0; i<demodulation_vector.size(); i++)
		printf("%c", demodulation_vector[i]);

	printf("\n");

	return true;
}

int SignalDemodulator::AppendDemodulationBit(int bit_value)
{
	if(demodulation_vector.size() == 0)
	{
		demodulation_char_bit_index = 0;
		demodulation_vector.push_back(0x00);
	}

	else if(demodulation_char_bit_index == 8)
	{
		demodulation_char_bit_index = 0;
		demodulation_vector.push_back(0x00);
	}

	demodulation_vector[demodulation_vector.size()-1] |= bit_value << demodulation_char_bit_index;
	demodulation_char_bit_index++;

	return true;
}

int SignalDemodulator::GatherSignalData()
{
	CyclicFifo* audio_input_fifo = port_audio_data->GetAudioInputFifo();

	complex<double> value = 0.00;
	while(audio_input_fifo->ReadData(value))
		signal_vector.push_back(value);

	return true;
}
