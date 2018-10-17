/*
  RFSniffer

  Usage: ./RFSniffer [<pulseLength>]
  [] = optional

  Hacked from http://code.google.com/p/rc-switch/
  by @justy to provide a handy RF code sniffer
*/

#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define Buffer_Size 128
#define Sync_Field_Size 32

RCSwitch mySwitch;

unsigned char decToCharValue(unsigned char decimal)
{
    static const unsigned char asciiChart[128] =
        { 
            0,      0,      0,      0,      0,      0,      0,      '\a',   '\b',   '\t',
            '\n',   '\v',   '\f',   '\r',   0,      0,      0,      0,      0,      0,
            0,      0,      0,      0,      0,      0,      0,      '\e',   0,      0,
            0,      0,      ' ',    '!',    '\"',   '#',    '$',    '%',    '&',    '\'',
            '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',    '0',    '1',
            '2',    '3',    '4',    '5',    '6',    '7',    '8',    '9',    ':',    ';',
            '<',    '=',    '>',    '\?',   '@',    'A',    'B',    'C',    'D',    'E',
            'F',    'G',    'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
            'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',    'X',    'Y',
            'Z',    '[',    '\\',   ']',    '^',    '_',    '`',    'a',    'b',    'c',
            'd',    'e',    'f',    'g',    'h',    'i',    'j',    'k',    'l',    'm',
            'n',    'o',    'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
            'x',    'y',    'z',    '{',    '|',    '}',    '~',    0
        };

    return asciiChart[decimal];
}
 
int main(int argc, char *argv[])
{
    // This pin is not the first pin on the RPi GPIO header!
    // Consult https://projects.drogon.net/raspberry-pi/wiringpi/pins/
    // for more information.
    int PIN = 2;

    if(wiringPiSetup() == -1)
    {
        printf("wiringPiSetup failed, exiting...");
        return 1;
    }

    // Default value of 1000
    int pulseLength = 1000;
    int confidence = 0;
    if (argc == 1)
    {
        printf("Usage: %s [pulselength] [corruption]\n", argv[0]);
        printf("pulselength\t- Pulselength in microseconds\n");
        printf("confidence\t- The percentage of confidence level\n");
        return -1;
    }
    
    if (argv[1] != NULL) pulseLength = atoi(argv[1]);
    if (argc >= 3) confidence = atoi(argv[2]);

    mySwitch = RCSwitch();
    mySwitch.setPulseLength(pulseLength);
    mySwitch.enableReceive(PIN);  // Receiver on interrupt 0 => that is pin #2

    // unsigned char syncFieldByteErrorCount = 0;
    // unsigned char bitwiseSyncByte = SYNC_FIELD_1;
    // unsigned char bufferByteCount = 0;
    // unsigned char buffer[MAX_BUFFER_BYTES];
    // memset(buffer, 0, sizeof(buffer));
    // bool syncFieldMatch = false;
    // bool syncByteMatch = false;

    int temp_match_percent[Buffer_Size - Sync_Field_Size] = {0};
    unsigned char input_buffer[Buffer_Size];
    char prefixes[2] = {0xA0, 0x50};
    unsigned char sync_field_buffer[Sync_Field_Size] = {0};
    int input_buffer_count, match_num, match_index = 0;
    int index;

    while(1)
    {
        if (mySwitch.available())
        {
            unsigned char value = (unsigned char)mySwitch.getReceivedValue();

            // if (value == 0) {
            //     printf("Unknown encoding\n");
            // } else {    
            //     // printf("Received %i\n", mySwitch.getReceivedValue() );
            //     printf("Received %i\n", value);
            // }

            // if (bufferByteCount < MAX_BUFFER_BYTES - 1)
            // {
            //     // if (value != '\0' || value != 4)
            //     if (value != 4)
            //     {
            //         printf("Received Byte: %u\n", value);
            //     }
            //     buffer[bufferByteCount] = value;
            //     bufferByteCount++;
            // }
            if (input_buffer_count < Buffer_Size)
            {
                if (value != 4)
                {
                    printf("Received Byte: %u (%c)\n", value, decToCharValue(value));
                }
                input_buffer[input_buffer_count] = value;
                input_buffer_count++;
            }
            else
            {
                input_buffer_count = 0;

                // Create Sync field for matching
                for(int i = 0; i < 2; i++)
                {
                    for(int j = 0; j < 16; j++)
                    {
                        index = i * 16 + j;
                        sync_field_buffer[index] = prefixes[i] + j;
                    }
                }
                
                for(int i = 0; i < Buffer_Size - Sync_Field_Size; i++)
                {
                    for(int j = 0; j < Sync_Field_Size; j++)
                    {
                        if(input_buffer[i + j] == sync_field_buffer[j]) 
                            match_num++;
                    }
                    temp_match_percent[i] = match_num;
                    if(temp_match_percent[match_index] < match_num) match_index = i;
                    match_num = 0;
                }
                
                int input_index = match_index + Sync_Field_Size;
                double match_percent = ( (double)temp_match_percent[match_index] / Sync_Field_Size ) * 100;

                if(match_percent >= confidence)
                {
                    printf("Success: Payload found with confidence level %d%% at index %d\n", confidence, input_index);
                    printf("Payload: ");
                    for(int i=match_index + Sync_Field_Size; i< match_index + Sync_Field_Size + 23; i++)    printf("%c", input_buffer[i]);
                    printf("\n");
                }
                else
                {
                    printf("Failed: Cannot find payload with confidence level %d%%\n", confidence);             
                }
            }

            fflush(stdout);
            mySwitch.resetAvailable();
        }

        usleep(500);
    }

    exit(0);
}
