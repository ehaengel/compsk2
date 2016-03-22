# compsk2

Installation instructions:
<br/>
After downloading the repository, from the command line cd into portaudio and type
<br/>
1) make clean <br/>
2) ./configure <br/>
3) make <br/>
4) cp lib/.libs/libportaudio.a ../lib <br/>
5) cd .. <br/>
6) make clean <br/>
7) make all <br/>
<br/>
After doing this, you should be able to run the compsk2 tool via the command ./main (port number)
<br/>
It is easiest to use netcat to communicate with the compsk2 tool. The following example is a good way to make sure that the software is working:
<br/>
ehaengel@puddles:~/Programming/current_projects/compsk2$ ./main 10000 <br/>
...
<br/>
ehaengel@puddles:~/Programming/current_projects/compsk2$ nc localhost 10000 <br/>
noise <br/>
quit <br/>
