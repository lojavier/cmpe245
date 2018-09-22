CC=arm-linux-gnueabihf-gcc
CFLAGS=

CXX=arm-linux-gnueabihf-g++
CXXFLAGS=-DRPI

LDFLAGS=

RM=rm -f

# all: clean send codesend RFSniffer Transmitter
all: clean codesend RFSniffer
# all: clean Transmitter

# send: RCSwitch.o send.o
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi -lwiringPiDev -lcrypt

codesend: RCSwitch.o codesend.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi -lwiringPiDev -lcrypt
	
RFSniffer: RCSwitch.o RFSniffer.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi -lwiringPiDev -lcrypt

# Transmitter: RFModule.o Transmitter.o
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lwiringPi -lwiringPiDev -lcrypt

clean:
	$(RM) *.o send codesend RFSniffer RFModule Transmitter
