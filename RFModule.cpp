#include "RFModule.h"

uint RFModule::mReceivedBitlength = 0;
uint RFModule::mReceivedDelay = 0;
uint RFModule::mReceivedProtocol = 0;
uint RFModule::mReceiveTolerance = 60;
uint RFModule::mTimings[RFMODULE_MAX_CHANGES];
unsigned long RFModule::mReceivedValue = 0;

RFModule::RFModule() 
{
	mReceiverInterrupt = -1;
	mTransmitterPin = -1;
	mReceivedValue = 0;
	setPulseLength(350);
	setRepeatTransmit(10);
	setReceiveTolerance(60);
	setProtocol(1);
}

RFModule::~RFModule() {}

// char* RFModule::dec2binWzerofill(string message, uint bitLength)
// {
// 	static char bin[64];
// 	uint i = 0;

// 	for (const char & c : message)
// 	{
// 	    payloadByte = (unsigned)c;
// 		cout << c << " = " << payloadByte << endl;
// 		mySwitch.send(payloadByte, 8); // 8 bits (1 byte) length
// 	}
	
// 	while (Dec > 0)
// 	{
// 		bin[32+i++] = ((Dec & 1) > 0) ? '1' : '0';
// 		Dec = Dec >> 1;
// 	}

// 	for (uint j = 0; j < bitLength; j++)
// 	{
// 		if (j >= bitLength - i)
// 		{
// 			bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
// 		}
// 		else
// 		{
// 			bin[j] = '0';
// 		}
// 	}
// 	bin[bitLength] = '\0';

// 	printf("%s\n", bin);

// 	return bin;
// }

char* RFModule::dec2binWzerofill(unsigned long Dec, uint bitLength)
{
	static char bin[64];
	uint i = 0;

	while (Dec > 0)
	{
		bin[32+i++] = ((Dec & 1) > 0) ? '1' : '0';
		Dec = Dec >> 1;
	}

	for (uint j = 0; j < bitLength; j++)
	{
		if (j >= bitLength - i)
		{
			bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
		}
		else
		{
			bin[j] = '0';
		}
	}
	bin[bitLength] = '\0';

	printf("%s\n", bin);

	return bin;
}

void RFModule::disableReceive()
{
	mReceiverInterrupt = -1;
}

void RFModule::enableReceive(int interrupt)
{
	mReceiverInterrupt = interrupt;
	enableReceive();
}

void RFModule::enableReceive()
{
	if (mReceiverInterrupt != -1)
	{
		mReceivedValue = 0;
		mReceivedBitlength = 0;
		wiringPiISR(mReceiverInterrupt, INT_EDGE_BOTH, &handleInterrupt);
	}
}

void RFModule::enableTransmit(int transmitterPin)
{
	mTransmitterPin = transmitterPin;
	pinMode(transmitterPin, OUTPUT);
}

void RFModule::handleInterrupt()
{
	static uint changeCount;
	static uint duration;
	static uint repeatCount;
	static unsigned long lastTime;
	
	unsigned long time = micros();
	duration = time - lastTime;

	if (duration > 5000 && duration > mTimings[0] - 200 && duration < mTimings[0] + 200)
	{    
		repeatCount++;
		changeCount--;

		if (repeatCount == 2)
		{
			// if (RFModule::receiveProtocol1(changeCount) == false)
			// {
			// 	// if (receiveProtocol2(changeCount) == false)
			// 	// {
			// 	// 	//failed
			// 	// 	printf("Receive of protocol 2 failed!\n");
			// 	// }
			// }
			repeatCount = 0;
		}
		changeCount = 0;
	} 
	else if (duration > 5000)
	{
		changeCount = 0;
	}

	if (changeCount >= RFMODULE_MAX_CHANGES)
	{
		changeCount = 0;
		repeatCount = 0;
	}
	mTimings[changeCount++] = duration;
	lastTime = time;
}

bool RFModule::receiveProtocol1(uint changeCount)
{
	unsigned long code = 0;
	unsigned long delay = mTimings[0] / 31;
	unsigned long delayTolerance = delay * mReceiveTolerance * 0.01;    

	for (int i = 1; i < changeCount; i+=2)
	{
		if (mTimings[i] > delay-delayTolerance && mTimings[i] < delay+delayTolerance && mTimings[i+1] > delay*3-delayTolerance && mTimings[i+1] < delay*3+delayTolerance)
		{
			code = code << 1;
		}
		else if (mTimings[i] > delay*3-delayTolerance && mTimings[i] < delay*3+delayTolerance && mTimings[i+1] > delay-delayTolerance && mTimings[i+1] < delay+delayTolerance)
		{
			code += 1;
			code = code << 1;
		}
		else 
		{
			// Failed
			i = changeCount;
			code = 0;
		}
	}

	code = code >> 1;

	if (changeCount > 6) // ignore < 4bit values as there are no devices sending 4bit values => noise
	{
		mReceivedValue = code;
		mReceivedBitlength = changeCount / 2;
		mReceivedDelay = delay;
		mReceivedProtocol = 1;
	}

	if (code == 0) 		return false;
	else if (code != 0)	return true;
}

void RFModule::send(unsigned long message, uint length)
{
	send( dec2binWzerofill(message, length) );
}

void RFModule::send(char* sCodeWord)
{
	for (uint repeat = 0; repeat < mRepeatTransmit; repeat++)
	{
		uint i = 0;
		while (sCodeWord[i] != '\0')
		{
			switch(sCodeWord[i])
			{
				case '0':
					send0();
					break;

				case '1':
					send1();
					break;

				default:
					break;
			}
			i++;
		}
		sendSync();
	}
}

void RFModule::send0()
{
	if (mProtocol == 1)
	{
		transmit(1,3);
	}
	// else if (mProtocol == 2)
	// {
	// 	transmit(1,2);
	// }
}

void RFModule::send1()
{
  	if (mProtocol == 1)
  	{
		transmit(3,1);
	}
	// else if (mProtocol == 2)
	// {
	// 	transmit(2,1);
	// }
}

void RFModule::sendSync()
{
    if (mProtocol == 1)
    {
		transmit(1,31);
	}
	// else if (mProtocol == 2)
	// {
	// 	this->transmit(1,10);
	// }
}

void RFModule::setProtocol(uint protocol)
{
	mProtocol = protocol;
}

void RFModule::setPulseLength(uint pulseLength)
{
	mPulseLength = pulseLength;
}

void RFModule::setReceiveTolerance(uint percent)
{
	mReceiveTolerance = percent;
}

void RFModule::setRepeatTransmit(uint repeatTransmit)
{
	mRepeatTransmit = repeatTransmit;
}

void RFModule::transmit(uint highPulses, uint lowPulses)
{
    bool disabled_receive = false;
    int receiverInterrupt_backup = mReceiverInterrupt;
    if (mTransmitterPin != -1)
    {
        if (mReceiverInterrupt != -1)
        {
            disableReceive();
            disabled_receive = true;
        }
        digitalWrite(mTransmitterPin, HIGH);
        delayMicroseconds(mPulseLength * highPulses);
        digitalWrite(mTransmitterPin, LOW);
        delayMicroseconds(mPulseLength * lowPulses);
        if(disabled_receive)
        {
            enableReceive(receiverInterrupt_backup);
        }
    }
}
