# Dockerized EnOceanSpy

Logs datagrams from a Eltako FAFT-60 to any url you want. Runs in Docker!

- build with `docker build -t docker-enoceanspy .`
- run with `docker run -l docker-enoceanspy-container -t -e "ENOCEAN_SPY_DEVICE=/dev/ttyUSB0" -e "ENOCEAN_SPY_URL=http://some.url" --device=/dev/ttyUSB0 docker-enoceanspy`

## or docker-compose (needs docker-compose version 1.3)

`docker-compose up`

## how to compile in docker
- if you start from alpine `apk add --update curl-dev gcc g++` then `gcc -c EnOceanSpy.c` then `gcc -lcurl -o EnOceanSpy EnOceanSpy.o`

# EnOceanSpy (Original Readme)

Use your Raspberry Pi to log all incoming EnOcean telegrams with an USB300 stick.

Requirements (hardware):
* Raspberry Pi
* EnOcean USB300 (Alternative: EnOcean Pi SoC-Gateway TRX 8051)


Compile EnOceanSpy with the following command

gcc -o EnOceanSpy EnOceanSpy.c

or use makefile with the following command

make

To use the EnOcean Pi SoC-Gateway TRX 8051 you need to disable Linux using the serial port for debugging. As default the serial port of the GPIO interface is used for console debug outputs. To use this port for your EnOcean Pi, this feature has to be disabled. There is a script available at GitHub to adapt boot up settings:
https://github.com/lurch/rpi-serial-console


As soon as compiling was succeesfull, you can start spying with

./EnOceanSpy /dev/ttyUSB0    (using EnOcean USB300)
or
./EnOceanSpy /dev/ttyAMA0    (using EnOcean Pi)

to log all imcoming telegrams at console or start it with

./EnOceanSpy <portname> > log.txt

to log all incoming telegrams in file 'log.txt'.


Have fun to seek your environment after EnOcean devices.
