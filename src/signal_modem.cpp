#include "signal_modem.h"

SignalModem::SignalModem()
{
	ResetModemData();

	//Application data
	port_audio_data = NULL;
	logfile_manager = NULL;
}

SignalModem::~SignalModem()
{
	//Do nothing
}

/////////////////////
// Data management //
/////////////////////

//Set SignalModem data
int SignalModem::SetSymbolPeriod(int symbol_period)
{
	this->symbol_period = symbol_period;

	return true;
}

int SignalModem::SetSymbolLength(int symbol_length)
{
	this->symbol_length = symbol_length;

	return true;
}

int SignalModem::SetDemodulateOn(int demodulate_on)
{
	this->demodulate_on = demodulate_on;

	return true;
}

int SignalModem::SetSynchronizationMessage(string synchronization_message)
{
	this->synchronization_message = synchronization_message;
	synchronization_transmit_signal.clear();
	synchronization_receive_signal.clear();

	return true;
}

int SignalModem::SetPortAudioData(PortAudioData* port_audio_data)
{
	this->port_audio_data = port_audio_data;

	return true;
}

int SignalModem::SetLogFileManager(LogFileManager* logfile_manager)
{
	this->logfile_manager = logfile_manager;

	return true;
}

//////////////////////
// COMPSK functions //
//////////////////////

int SignalModem::Initialize()
{
	if(LoadConfigurationFile() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not load configuration file");
		return false;
	}

	return true;
}

int SignalModem::LoadConfigurationFile()
{
	XML_Document* xml_document = new XML_Document;
	if(xml_document->LoadFromFile("data/configuration.xml") == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not load configuration file");
		return false;
	}

	for(size_t i=0; i<xml_document->GetContentCount(); i++)
	{
		XML_Content* xml_content = xml_document->GetContent(i);
		if(xml_content->GetType() != XML_Content::TAG_CONTENT)
			continue;

		if(compare_lowercase(xml_content->GetTagName(), "ModemConfiguration"))
		{
			if(LoadModemConfiguration(xml_content) == false)
			{
				logfile_manager->WriteErrorMessage("SignalModem", "Could not load modem configuration");
				return false;
			}
		}
	}

	delete xml_document;
	return true;
}

int SignalModem::RunModem()
{
	if(demodulate_on == false)
		return true;

	//Create all necessary signals
	if(synchronization_transmit_signal.size() == 0 && CreateSynchronizationSignal() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not create synchronization signal");
		return false;
	}

	//Collect signal data
	if(CollectSignalData() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not collect signal data");
		return false;
	}

	if(DemodulateSignal() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not demodulate signal data");
		return false;
	}

	//Clean up all signal vectors
	if(CleanupSignalVectors() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not clean up signal vectors");
		return false;
	}

	return true;
}

int SignalModem::TransmitMessage(BitList transmission_message)
{
	if(InitializeTransmissionSignal() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not initialize transmission signal");
		return false;
	}

	for(size_t i=0; i<transmission_message.size(); i++)
		SynthesizeBit(transmission_signal, transmission_message[i]);

	//Transmit signal over audio
	port_audio_data->SetAudioMode(PortAudioData::STANDBY_MODE);

	CyclicFifo* audio_output_fifo = port_audio_data->GetAudioOutputFifo();

	for(size_t i=0; i<transmission_signal.size(); i++)
		audio_output_fifo->WriteData(transmission_signal[i]);

	printf("transmission_signal.size() = %lu\n", transmission_signal.size());
	transmission_signal.clear();

	return true;
}

int SignalModem::TransmitMessage(string transmission_message)
{
	this->transmission_message = transmission_message;

	if(SendTransmissionSignal() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not transmit message");
		return false;
	}

	return true;
}

int SignalModem::WriteDataUsage(FILE* client_handle)
{
	fprintf(client_handle, "synchronization_bit_list.size() = %lu\n", synchronization_bit_list.size());
	fprintf(client_handle, "synchronization_constellation_point_list.size() = %lu\n", synchronization_constellation_point_list.size());
	fprintf(client_handle, "synchronization_transmit_signal.size() = %lu\n", synchronization_transmit_signal.size());
	fprintf(client_handle, "synchronization_receive_signal.size() = %lu\n", synchronization_receive_signal.size());
	fprintf(client_handle, "received_signal.size() = %lu\n", received_signal.size());
	fprintf(client_handle, "constellation_point_fifo.size() = %lu\n", constellation_point_fifo.size());
	fprintf(client_handle, "transmission_signal.size() = %lu\n", transmission_signal.size());
	fflush(client_handle);

	return true;
}

/////////////////////////
// Debugging functions //
/////////////////////////

int SignalModem::Print()
{

	return true;
}

int SignalModem::SaveSynchronizationSignal()
{
	FILE* handle_tx = fopen("data/synchronization_transmit_signal.dat", "w");
	FILE* handle_rx = fopen("data/synchronization_receive_signal.dat", "w");

	for(size_t i=0; i<synchronization_transmit_signal.size(); i++)
		fprintf(handle_tx, "%.8lf %.8lf\n", real(synchronization_transmit_signal[i]), imag(synchronization_transmit_signal[i]));

	for(size_t i=0; i<synchronization_receive_signal.size(); i++)
		fprintf(handle_rx, "%.8lf %.8lf\n", real(synchronization_receive_signal[i]), imag(synchronization_receive_signal[i]));

	fclose(handle_tx);
	fclose(handle_rx);
	return true;
}

////////////////////////////
// Internal use functions //
////////////////////////////

//Initialization functions
int SignalModem::CreateSynchronizationSignal()
{
	synchronization_transmit_signal.clear();
	synchronization_receive_signal.clear();

	FILE* handle = fopen("data/new_sync_code.dat", "r");
	if(handle == NULL)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not load new_sync_code.dat");
		return false;
	}

	char buffer[1000];
	fgets(buffer, 1000, handle);

	while(!feof(handle))
	{
		int bit = 0;
		sscanf(buffer, "%d", &bit);

		ConstellationPoint constellation_point = double(bit);

		synchronization_bit_list.push_back(bit);
		synchronization_constellation_point_list.push_back(constellation_point);

		synchronization_receive_signal.push_back(double(bit));

		if(SynthesizeBit(synchronization_transmit_signal, bit) == false)
		{
			logfile_manager->WriteErrorMessage("SignalModem", "Could not synthesize bit");
			return false;
		}

		fgets(buffer, 1000, handle);
	}

	printf("synchronization_transmit_signal.size() = %lu\n", synchronization_transmit_signal.size());

	fclose(handle);

	return true;
}

//Receiver functions
int SignalModem::DemodulateSignal()
{
	if(received_signal.size() == 0)
		return true;

	ConstellationPointList result_list;

	double I_result = 0.00;
	double Q_result = 0.00;
	double omega = 2 * M_PI / double(symbol_period);

	double magnitude = 0.00;

	size_t index = 0;
	while(index + (2*symbol_length*symbol_period) < received_signal.size())
	{
		I_result = 0.00;
		Q_result = 0.00;
		for(size_t i=index; i<index+(symbol_length*symbol_period); i++)
		{
			I_result = I_result + real(received_signal[i] * sin(i*omega + phase_index));
			Q_result = Q_result + real(received_signal[i] * cos(i*omega + phase_index));
		}

		magnitude = magnitude + (I_result*I_result + Q_result*Q_result);

		result_list.push_back(complex<double>(I_result, Q_result));

		index = index + symbol_length*symbol_period;

		//phase_index = phase_index - omega/150.00;
	}
	//printf("phase_index = %.5lf\n", phase_index);

	received_signal.erase(received_signal.begin(), received_signal.begin()+index);

	magnitude = 10*log10(magnitude / double(result_list.size()));
	printf("Magnitude = %.2lf\n", magnitude);

	constellation_point_fifo.insert(constellation_point_fifo.end(), result_list.begin(), result_list.end());

	//if(magnitude > 10.00)
	{
		BitList bit_list;
		UnsynthesizeConstellationPoints(bit_list, constellation_point_fifo);

		FILE* handle = fopen("data/demodulate.dat", "w");
		for(size_t i=0; i<constellation_point_fifo.size(); i++)
			fprintf(handle, "%.8lf %.8lf\n", real(constellation_point_fifo[i]), imag(constellation_point_fifo[i]));

		fclose(handle);
	}

	return true;
}

/*int SignalModem::DemodulateSignal(size_t message_start_index, size_t& message_stop_index)
{
	received_message = "";

	double omega = 2 * M_PI / double(symbol_period);
	size_t index = message_start_index;
	while(index < received_signal.size())
	{
		char c = 0;
		for(int i=0; i<8; i++)
		{
			complex<double> correlation = 0.00;
			for(int j=0; j<symbol_period*symbol_length; j++)
				correlation = correlation + received_signal[index + i*symbol_period*symbol_length + j] * sin(j * omega);

			if(real(correlation) > 0)
				c = c + (1 << i);
		}
		received_message = received_message + c;

		index = index + symbol_period*symbol_length*8;
	}

	printf("%s\n", received_message.c_str());

	return true;
}*/

int SignalModem::CollectSignalData()
{
	CyclicFifo* audio_input_fifo = port_audio_data->GetAudioInputFifo();

	complex<double> z = 0.00;
	while(audio_input_fifo->ReadData(z))
		received_signal.push_back(z);

	printf("received_signal.size() = %lu\n", received_signal.size());

	return true;
}

int SignalModem::CleanupSignalVectors()
{
	//received_signal.clear();

	if(constellation_point_fifo.size() > 5400)
	{
		size_t margin = constellation_point_fifo.size() - 5400;
		constellation_point_fifo.erase(constellation_point_fifo.begin(), constellation_point_fifo.begin() + margin);
	}

	return true;
}

//Transmission functions
int SignalModem::InitializeTransmissionSignal()
{
	transmission_signal.clear();
	transmission_signal.insert(transmission_signal.begin(), synchronization_transmit_signal.begin(), synchronization_transmit_signal.end());

	return true;
}

int SignalModem::AppendToTransmissionSignal(string str)
{
	for(size_t i=0; i<str.length(); i++)
	{
		if(SynthesizeByte(transmission_signal, str[i]) == false)
		{
			logfile_manager->WriteErrorMessage("SignalModem", "Could not synthesize byte");
			return false;
		}
	}

	return true;
}

int SignalModem::SendTransmissionSignal()
{
	if(InitializeTransmissionSignal() == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not initialize transmission signal");
		return false;
	}

	if(AppendToTransmissionSignal(transmission_message) == false)
	{
		logfile_manager->WriteErrorMessage("SignalModem", "Could not append message to transmission signal");
		return false;
	}
	transmission_message = "";

	//Transmit signal over audio
	port_audio_data->SetAudioMode(PortAudioData::STANDBY_MODE);

	CyclicFifo* audio_output_fifo = port_audio_data->GetAudioOutputFifo();

	for(size_t i=0; i<transmission_signal.size(); i++)
		audio_output_fifo->WriteData(transmission_signal[i]);

	printf("transmission_signal.size() = %lu\n", transmission_signal.size());
	transmission_signal.clear();

	//port_audio_data->SetAudioMode(PortAudioData::PLAY_MODE);

	return true;
}

//General purpose functions
int SignalModem::SynthesizeByte(SignalVector& signal_vector, char c)
{
	//size_t initial_size = signal_vector.size();
	//signal_vector.resize(initial_size + 8*symbol_period*symbol_length, 0.00);

	double omega = 2 * M_PI / double(symbol_period);
	for(int i=0; i<8; i++)
	{
		if(SynthesizeBit(signal_vector, 2*((c >> i) & 0x01)-1) == false)
			return false;
	}

	return true;
}

int SignalModem::SynthesizeBit(SignalVector& signal_vector, int b)
{
	size_t initial_size = signal_vector.size();
	signal_vector.resize(initial_size + symbol_period*symbol_length, 0.00);

	double multiplier = 1.00;
	if(b == -1)
		multiplier = -1.00;

	double duration = symbol_period*symbol_length;
	double center_i = symbol_period*symbol_length/2.0;

	printf("b = %d, %.lf\n", b, multiplier);
	double omega = 2 * M_PI / double(symbol_period);
	for(int i=0; i<symbol_period*symbol_length; i++)
		signal_vector[initial_size + i] = multiplier*sin(omega*i)*cos(M_PI*(double(i) - center_i)/duration);

	return true;
}

int SignalModem::SynthesizeConstellationPoints(SignalVector& signal_vector, char c)
{
	for(int i=0; i<8; i++)
	{
		double multiplier = 1.00;
		if(((c >> i) & 0x01) == 0x00)
			multiplier = -1.00;

		signal_vector.push_back(multiplier);
	}

	return true;
}

/*int SignalModem::UnsynthesizeConstellationPoints(string& message, SignalVector signal_vector)
{
	int index = 0;
	char c = 0;
	while(signal_vector.size() > 0)
	{
		complex<double> value = signal_vector[0];
		signal_vector.erase(signal_vector.begin());

		if(abs(value) > 10)
		{
			if(abs(value + 1.00) > abs(value - 1.00))
				c = c + (1 << index);

			index++;
		}

		if(index == 8)
		{
			if(c != 0)
			{
				message = message + c;
				c = 0;
			}

			index = 0;
		}
	}

	return true;
}*/

int SignalModem::UnsynthesizeConstellationPoints(BitList& bit_list, ConstellationPointList constellation_point_list)
{
	//Find synchronization point
	size_t max_correlation_index = 0;
	complex<double> max_correlation = 0.00;

	for(size_t i=0; i+synchronization_constellation_point_list.size()<constellation_point_list.size(); i++)
	{
		complex<double> correlation = 0.00;
		for(size_t k=0; k<synchronization_constellation_point_list.size(); k++)
			correlation += conj(synchronization_constellation_point_list[k]) * constellation_point_list[i+k];

		if(abs(correlation) > abs(max_correlation))
		{
			max_correlation = correlation;
			max_correlation_index = i;
		}
		//printf("%lu %.5f %.5f %.5f\n", i, real(correlation), imag(correlation), abs(correlation));
	}
	printf("max_correlation = %.5lf\n", abs(max_correlation));
	max_correlation = max_correlation / abs(max_correlation);

	for(size_t i=0; i<constellation_point_list.size(); i++)
	{
		constellation_point_list[i] = constellation_point_list[i] / max_correlation;

		if(abs(constellation_point_list[i]) > 10.0)
		{
			if(real(constellation_point_list[i]) > 0.00)
				bit_list.push_back(0x01);

			else
				bit_list.push_back(0x00);
		}
	}

	FILE* handle = fopen("data/demodulate_fixed.dat", "w");

	for(size_t i=0; i<constellation_point_list.size(); i++)
		fprintf(handle, "%.8lf %.8lf\n", real(constellation_point_list[i]), imag(constellation_point_list[i]));

	fclose(handle);

	handle = fopen("data/bit_list.dat", "w");

	for(size_t i=0; i<bit_list.size(); i++)
		fprintf(handle, "%d\n", int(bit_list[i]));

	fclose(handle);


	return true;
}

int SignalModem::ResetModemData()
{
	//Modulation data
	symbol_period = 49;
	symbol_length = 10;

	demodulate_on = false;

	//Synchronization data
	synchronization_bit_list.clear();
	synchronization_constellation_point_list.clear();

	synchronization_message = "r";
	synchronization_transmit_signal.clear();
	synchronization_receive_signal.clear();

	//Receiver data
	received_signal.clear();
	received_message = "";

	constellation_point_fifo.clear();

	phase_index = 0.00;

	//Transmitter data
	transmission_message = "";
	transmission_signal.clear();

	return true;
}

int SignalModem::LoadModemConfiguration(XML_Content* modem_configuration_tag)
{
	for(size_t i=0; i<modem_configuration_tag->GetChildCount(); i++)
	{
		XML_Content* xml_content = modem_configuration_tag->GetChild(i);
		if(xml_content->GetType() != XML_Content::TAG_CONTENT)
			continue;

		if(compare_lowercase(xml_content->GetTagName(), "SymbolPeriod"))
		{
			if(xml_content->GetChildCount() != 1 || xml_content->GetChild(0)->GetType() != XML_Content::TEXT_CONTENT)
			{
				logfile_manager->WriteErrorMessage("SignalModem", "Invalid SymbolPeriod tag");
				return false;
			}
			XML_Content* child_content = xml_content->GetChild(0);

			sscanf(child_content->GetText().c_str(), "%d", &symbol_period);
		}

		else if(compare_lowercase(xml_content->GetTagName(), "SymbolLength"))
		{
			if(xml_content->GetChildCount() != 1 || xml_content->GetChild(0)->GetType() != XML_Content::TEXT_CONTENT)
			{
				logfile_manager->WriteErrorMessage("SignalModem", "Invalid SymbolLength tag");
				return false;
			}
			XML_Content* child_content = xml_content->GetChild(0);

			sscanf(child_content->GetText().c_str(), "%d", &symbol_length);
		}

		else if(compare_lowercase(xml_content->GetTagName(), "SynchronizationMessage"))
		{
			if(xml_content->GetChildCount() != 1 || xml_content->GetChild(0)->GetType() != XML_Content::TEXT_CONTENT)
			{
				logfile_manager->WriteErrorMessage("SignalModem", "Invalid SynchronizationMessage tag");
				return false;
			}
			XML_Content* child_content = xml_content->GetChild(0);

			synchronization_message = child_content->GetText();
		}
	}

	return true;
}
