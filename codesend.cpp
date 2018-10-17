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
#include <time.h>

#define Buffer_Size 128
#define Sync_Field_Size 32

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
	int corruption = 0;

    // If no command line argument is given, print the help text
    if (argc == 1) {
		printf("Usage: %s [pulselength] [corruption]\n", argv[0]);
		printf("pulselength\t- Pulselength in microseconds\n");
		printf("corruption\t- The percentage of corruption\n");
		return -1;
    }

    // Change protocol and pulse length accroding to parameters
    // int code = atoi(argv[1]);
    pulseLength = atoi(argv[1]);
    if (argc >= 3) corruption = atoi(argv[2]);
	const char * payload = "SJSU_CMPE245_LORENZO_9623";

    if (wiringPiSetup () == -1) return 1;
    // printf("sending code[%i]\n", code);
    RCSwitch mySwitch = RCSwitch();
    if (protocol != 0) mySwitch.setProtocol(protocol);
    if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);
    mySwitch.enableTransmit(PIN);
    // mySwitch.send(code, 24);

    // Send Sync Field Part 1 = 0xA0 -> 0xAF
  //   for(uint syncByte = 160; syncByte < 176; syncByte++)
  //   {
		// mySwitch.send(syncByte, 8); // 8 bits (1 byte) length
		// printf("Transmit Sync Byte: (0x%c%c) %d\n", decToHexValue((syncByte >> 4) & 0x0F), decToHexValue(syncByte & 0x0F), syncByte);
  //   }
  //   // Send Sync Field Part 2 = 0x50 -> 0x5F
  //   for(uint syncByte = 80; syncByte < 96; syncByte++)
  //   {
		// mySwitch.send(syncByte, 8); // 8 bits (1 byte) length
		// printf("Transmit Sync Byte: (0x%c%c) %d\n", decToHexValue((syncByte >> 4) & 0x0F), decToHexValue(syncByte & 0x0F), syncByte);
  //   }

  //   // Send Payload w/offset
  //   uint c = 0;
  //   uint payloadByte = 0;
  //   // char payload[MAX_PAYLOAD_BYTES] = "SJSU_CMPE245_LORENZO_9623";
  //   while (payload[c] != '\0')
  //   {
  //   	payloadByte = (unsigned)payload[c];
		// mySwitch.send(payloadByte, 8); // 8 bits (1 byte) length
		// printf("Transmit Payload Byte: (%c) = %d\n", payload[c], payloadByte);
		// c++;
  //   }

  //   mySwitch.setPulseLength(500);
  //   // Send remaining bytes to fill up 128 byte buffer on reciever end
  //   for(uint i = (unsigned)strlen(payload); i < MAX_PAYLOAD_BYTES+10; i++)
  //   {
  //   	mySwitch.send(4, 8); // 4 EOT (end of transmission)
  //   	printf("Transmit Payload Byte: (EOT) = 4\n");
  //   }

    srand(time(NULL));
    int sync_field_index, payload_index;
    unsigned char sync_field_buffer[Sync_Field_Size] = {0};
    unsigned char output_buffer[Buffer_Size];
    for(int k = 0; k < Buffer_Size; k++) output_buffer[k] = rand() % (256)+1;
    
    char prefixes[2] = {0xA0, 0x50};
    int corrupt_num = (Sync_Field_Size * corruption) / 100;
    int corrupted_record [corrupt_num];
    int rand_byte = 0;
    int index;

    // Create Sync field
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 16; j++)
        {
            index = i * 16 + j;
            sync_field_buffer[index] = prefixes[i] + j;
        }
    }

    // Corrupt Sync field
    for(int k = 0; k < corrupt_num; k++)    
        corrupted_record [k] = {-1};

    for(int i = 0; i < corrupt_num; i++)
    {
run:
        rand_byte = rand() % (Sync_Field_Size);
        for(int j = 0; j < corrupt_num; j++)
        {
            if(corrupted_record[j] == rand_byte)   
                goto run;
        }
        corrupted_record[i] = rand_byte;
        sync_field_buffer[rand_byte] = rand() % (Sync_Field_Size * 8);
    }
    
    int payload_length = strlen(payload);
    sync_field_index = rand() % (Buffer_Size - (Sync_Field_Size + payload_length));
    payload_index = Sync_Field_Size + sync_field_index;
    for(int i = sync_field_index; i < sync_field_index + Sync_Field_Size; i++)  //insert sync field + payload at random offset
    {
        output_buffer[i] = sync_field_buffer[i - sync_field_index];
        output_buffer[i + Sync_Field_Size] = payload [i - sync_field_index];
    }

    // Send output
    for(int i = 0; i < Buffer_Size; i++)
    {
        mySwitch.send(output_buffer[i], 8);
        printf("Transmit Byte: (0x%02X) = %d\n", output_buffer[i], output_buffer[i]);
    }

    for(int i = 0; i < Buffer_Size / 4; i++)
    {
        mySwitch.send(4, 8); // 4 EOT (end of transmission)
        printf("Transmit Byte: (EOT) = 4\n");
    }
    
    return 0;
}
