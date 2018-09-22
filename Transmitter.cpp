#include "RFModule.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream> 
#include <string>

using namespace std;

int main(int argc, char *argv[]) 
{
	if (wiringPiSetup() == -1) 
	{
		printf("Wiring Pi not setup!\n");
		return 1;
	}

    uint message = atoi(argv[1]);  //Code is what you received from RFSniffer
	uint pulseLength = atoi(argv[2]); //Pulse is what you received from RFSniffer
	uint syncDec1 = 160;		// 0xA0 (10100000) 32+128=160
	uint syncDec2 = 80;			// 0x50 (01010000) 16+64=80
	const char payloadString[256] = "SJSU_CMPE245_Lorenzo_9623";

	// RFModule RFTX0;
	// RFTX0.setPulseLength(pulseLength);
	// RFTX0.enableTransmit(WiringPiPin0);
	// RFTX0.send(message, 24);	// max length = 24 bits

	// for (const char & c : payloadString)
	// {
	//     payloadByte = (unsigned)c;
	// 	cout << c << " = " << payloadByte << endl;
	// 	mySwitch.send(payloadByte, 8); // 8 bits (1 byte) length
	// }

	uint short_delay = pulseLength;
	uint long_delay = pulseLength * 3;
	uint extended_delay = pulseLength * 31;

	uint transmitterPin = WiringPiPin0;
	pinMode(transmitterPin, OUTPUT);
	// digitalWrite(transmitterPin, LOW);
	// delayMicroseconds(extended_delay);

	char code = 'A';
	uint dec = (unsigned)code;
	printf("%c (%d) = ", code, dec);
	for(int j = 0; j < 10; j++)
	{
		// for(int i = 7; i >= 0; i--)
		// {
		// 	if((dec >> i) & 0x1)
		// 	{
		// 		digitalWrite(transmitterPin, HIGH);
		// 		delayMicroseconds(short_delay);
		// 	    digitalWrite(transmitterPin, LOW);
		// 	    delayMicroseconds(long_delay);
		// 	    printf("1 ");
		// 	}
		// 	else
		// 	{
		// 		digitalWrite(transmitterPin, HIGH);
		// 		delayMicroseconds(long_delay);
		// 	    digitalWrite(transmitterPin, LOW);
		// 	    delayMicroseconds(short_delay);
		// 	    printf("0 ");
		// 	}
		// }
		digitalWrite(transmitterPin, HIGH);
		delayMicroseconds(long_delay);
	    digitalWrite(transmitterPin, LOW);
	    delayMicroseconds(short_delay);
	    // printf("1 ");
		for(int i = 7; i >= 0; i--)
		{
			if((dec >> i) & 0x1)
			{
				digitalWrite(transmitterPin, HIGH);
				delayMicroseconds(long_delay);
			    digitalWrite(transmitterPin, LOW);
			    delayMicroseconds(short_delay);
			    // printf("1 ");
			}
			else
			{
				digitalWrite(transmitterPin, HIGH);
				delayMicroseconds(short_delay);
			    digitalWrite(transmitterPin, LOW);
			    delayMicroseconds(long_delay);
			    // printf("0 ");
			}
		}
		printf("\n");
		delayMicroseconds(extended_delay);
	}

	printf("%s\n", to_string(dec).c_str());

	// for(uint i = 300; i < 900; i+=50)
	// {
	// 	printf("delay = %d\n", i);
	// 	for(uint j = 0; j < 20; j++)
	// 	{
	// 		digitalWrite(transmitterPin, HIGH);
	// 		delay(i);
	// 	    digitalWrite(transmitterPin, LOW);
	// 	    delay(i);
	// 	}
	// }

	return 0;
}
