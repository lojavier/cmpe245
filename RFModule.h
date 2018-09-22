#ifndef RFMODULE_H
#define RFMODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

#define WiringPiPin0 0	// Wiring Pi pin 0 | BCM pin 17 | Physical pin 11
#define WiringPiPin2 2	// Wiring Pi pin 2 | BCM pin 27 | Physical pin 13

#define CHANGE 1
#define RFMODULE_MAX_CHANGES 67

class RFModule {

	public:
		RFModule();
		~RFModule();

		void disableReceive();
		void enableReceive(int);
		void enableReceive();
		void enableTransmit(int);
		void send(unsigned long, uint);
		void send(char *);
		void setProtocol(uint);
		void setPulseLength(uint);
		void setReceiveTolerance(uint);
		void setRepeatTransmit(uint);
		
	private:
		void send0();
    	void send1();
		void sendSync();
		void transmit(uint, uint);
		static bool receiveProtocol1(uint);
		static bool receiveProtocol2(uint);
		static char* dec2binWzerofill(unsigned long, uint);
	    static void handleInterrupt();

	    int mReceiverInterrupt;
	    int mTransmitterPin;
		uint mProtocol;
		uint mPulseLength;
		uint mRepeatTransmit;
	    static uint mReceivedBitlength;
		static uint mReceivedDelay;
		static uint mReceivedProtocol;
		static uint mReceiveTolerance;
	    static uint mTimings[RFMODULE_MAX_CHANGES];
	    static unsigned long mReceivedValue;

};

#endif