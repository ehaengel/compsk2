COMPSK_TARGETS	=	lib/cyclic_fifo.o \
					lib/port_audio_data.o \
					lib/audio_server.o \
					lib/signal_modem.o \
					lib/signal_modulator.o \
					lib/signal_demodulator.o \
					lib/communication_server.o

all:	main

# COMPSK library
lib/cyclic_fifo.o: src/cyclic_fifo.h src/cyclic_fifo.cpp
	g++ src/cyclic_fifo.cpp -c -o lib/cyclic_fifo.o

lib/port_audio_data.o: src/port_audio_data.h src/port_audio_data.cpp
	g++ src/port_audio_data.cpp -c -o lib/port_audio_data.o

lib/audio_server.o: src/audio_server.h src/audio_server.cpp
	g++ src/audio_server.cpp -c -o lib/audio_server.o

lib/signal_modem.o: src/signal_modem.h src/signal_modem.cpp
	g++ src/signal_modem.cpp -c -o lib/signal_modem.o

lib/signal_modulator.o: src/signal_modulator.h src/signal_modulator.cpp
	g++ src/signal_modulator.cpp -c -o lib/signal_modulator.o

lib/signal_demodulator.o: src/signal_demodulator.h src/signal_demodulator.cpp
	g++ src/signal_demodulator.cpp -c -o lib/signal_demodulator.o

lib/communication_server.o: src/communication_server.h src/communication_server.cpp
	g++ src/communication_server.cpp -c -o lib/communication_server.o

# COMPSK library
lib/libcompsk.a: $(COMPSK_TARGETS)
	ar -cr lib/libcompsk.a $(COMPSK_TARGETS)

# Main
main: src/main.cpp lib/libcompsk.a
	g++ src/main.cpp -Llib -lcompsk -llogfile -lsimplexml2 lib/libportaudio.a -lpthread -lasound -ljack -o main

# Clean
clean:
	rm -f lib/*.o lib/libcompsk.a main

