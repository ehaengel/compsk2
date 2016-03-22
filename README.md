# compsk2

Installation instructions:

After downloading the repository, from the command line cd into portaudio and type

1) make clean
2) ./configure
3) make
4) cp lib/.libs/libportaudio.a ../lib
5) cd ..
6) make clean
7) make all

After doing this, you should be able to run the compsk2 tool via the command ./main (port number)

It is easiest to use netcat to communicate with the compsk2 tool. The following example is a good way to make sure that the software is working:

ehaengel@puddles:~/Programming/current_projects/compsk2$ ./main 10000
...

ehaengel@puddles:~/Programming/current_projects/compsk2$ nc localhost 10000
noise
quit
