/*
Usage: ./codesend decimalcode [protocol] [pulselength]
decimalcode - As decoded by RFSniffer
protocol    - According to rc-switch definitions
pulselength - pulselength in microseconds

 'codesend' hacked from 'send' by @justy
 
 - The provided rc_switch 'send' command uses the form systemCode, unitCode, command
   which is not suitable for our purposes.  Instead, we call 
   send(code, length); // where length is always 24 and code is simply the code
   we find using the RF_sniffer.ino Arduino sketch.

(Use RF_Sniffer.ino to check that RF signals are being produced by the RPi's transmitter 
or your remote control)
*/
#include "RCSwitch.h"

char decToHexValue(uint decimal)
{
	char ch;
	switch(decimal)
	{
		case 0:		ch = '0'; break;
		case 1:		ch = '1'; break;
		case 2:		ch = '2'; break;
		case 3:		ch = '3'; break;
		case 4:		ch = '4'; break;
		case 5:		ch = '5'; break;
		case 6:		ch = '6'; break;
		case 7:		ch = '7'; break;
		case 8:		ch = '8'; break;
		case 9:		ch = '9'; break;
		case 10:	ch = 'A'; break;
		case 11:	ch = 'B'; break;
		case 12:	ch = 'C'; break;
		case 13:	ch = 'D'; break;
		case 14:	ch = 'E'; break;
		case 15:	ch = 'F'; break;
		default:	ch = 'x'; break;
	}
	return ch;
}
     
int main(int argc, char *argv[]) {
    
    // This pin is not the first pin on the RPi GPIO header!
    // Consult https://projects.drogon.net/raspberry-pi/wiringpi/pins/
    // for more information.
    int PIN = 0;
    
    // Parse the first parameter to this command as an integer
    int protocol = 0; // A value of 0 will use rc-switch's default value
    int pulseLength = 0;

    // If no command line argument is given, print the help text
    if (argc == 1) {
        printf("Usage: %s decimalcode [protocol] [pulselength]\n", argv[0]);
        printf("decimalcode\t- As decoded by RFSniffer\n");
        printf("protocol\t- According to rc-switch definitions\n");
        printf("pulselength\t- pulselength in microseconds\n");
        return -1;
    }

    // Change protocol and pulse length accroding to parameters
    int code = atoi(argv[1]);
    if (argc >= 3) protocol = atoi(argv[2]);
    if (argc >= 4) pulseLength = atoi(argv[3]);
    
    if (wiringPiSetup () == -1) return 1;
    // printf("sending code[%i]\n", code);
    RCSwitch mySwitch = RCSwitch();
    if (protocol != 0) mySwitch.setProtocol(protocol);
    if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);
    mySwitch.enableTransmit(PIN);
    // mySwitch.send(code, 24);

    // Send Sync Field Part 1 = 0xA0 -> 0xAF
    for(uint syncByte = 160; syncByte < 176; syncByte++)
    {
		mySwitch.send(syncByte, 8); // 8 bits (1 byte) length
		printf("Transmit Sync Byte: (0x%c%c) %d\n", decToHexValue((syncByte >> 4) & 0x0F), decToHexValue(syncByte & 0x0F), syncByte);
    }
    // Send Sync Field Part 2 = 0x50 -> 0x5F
    for(uint syncByte = 80; syncByte < 96; syncByte++)
    {
		mySwitch.send(syncByte, 8); // 8 bits (1 byte) length
		printf("Transmit Sync Byte: (0x%c%c) %d\n", decToHexValue((syncByte >> 4) & 0x0F), decToHexValue(syncByte & 0x0F), syncByte);
    }

    // Send Payload w/offset
    uint c = 0;
    uint payloadByte = 0;
    char payloadString[MAX_PAYLOAD_BYTES] = "SJSU_CMPE245_LORENZO_9623";
    while (payloadString[c] != '\0')
    {
    	payloadByte = (unsigned)payloadString[c];
		mySwitch.send(payloadByte, 8); // 8 bits (1 byte) length
		printf("Transmit Payload Byte: (%c) = %d\n", payloadString[c], payloadByte);
		c++;
    }

    mySwitch.setPulseLength(500);
    // Send remaining bytes to fill up 128 byte buffer on reciever end
    for(uint i = (unsigned)strlen(payloadString); i < MAX_PAYLOAD_BYTES+10; i++)
    {
    	mySwitch.send(4, 8); // 4 EOT (end of transmission)
    	printf("Transmit Payload Byte: (EOT) = 4\n");
    }
    
    return 0;
}
