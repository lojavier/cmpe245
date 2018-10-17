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

    int pulseLength = 0;
    if (argv[1] != NULL) pulseLength = atoi(argv[1]);

    mySwitch = RCSwitch();
    if (pulseLength != 0) mySwitch.setPulseLength(pulseLength);
    mySwitch.enableReceive(PIN);  // Receiver on interrupt 0 => that is pin #2

    unsigned char syncFieldByteErrorCount = 0;
    unsigned char bitwiseSyncByte = SYNC_FIELD_1;
    unsigned char bufferByteCount = 0;
    unsigned char buffer[MAX_BUFFER_BYTES];
    memset(buffer, 0, sizeof(buffer));
    // bool syncFieldMatch = false;
    // bool syncByteMatch = false;

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

            if (bufferByteCount < MAX_BUFFER_BYTES - 1)
            {
                // if (value != '\0' || value != 4)
                if (value != 4)
                {
                    printf("Received Byte: %u\n", value);
                }
                buffer[bufferByteCount] = value;
                bufferByteCount++;
            }
            else
            {
                buffer[bufferByteCount] = value;
                bufferByteCount = 0;

                for (int j = 0; j < MAX_SYNC_BYTES; j++)
                {
                    printf("%u ^ %u = %u\n", buffer[j], bitwiseSyncByte, (buffer[j] ^ bitwiseSyncByte));

                    int x = bitwiseSyncByte - buffer[j];
                    // if (buffer[j] ^ bitwiseSyncByte)
                    if (x != 0)
                    {
                        // j--;
                        j -= (bitwiseSyncByte - buffer[j]);
                        syncFieldByteErrorCount++;
                    }

                    // bitwiseSyncByte++;
                    bitwiseSyncByte += x;

                    if (bitwiseSyncByte == 176)
                    {
                        bitwiseSyncByte = SYNC_FIELD_2;
                    }
                    else if (bitwiseSyncByte == 96)
                    {
                        break;
                    }
                }

                printf("Sync Field Byte Error Count: %u\n", syncFieldByteErrorCount);

                for (uint i = (MAX_SYNC_BYTES - syncFieldByteErrorCount); i < MAX_BUFFER_BYTES; i++)
                {
                    if (buffer[i] == '\0' || buffer[i] == 4)
                    {
                        break;
                    }
                    else
                    {
                        printf("%c", decToCharValue(buffer[i]));
                    }
                }
                printf("\n");

                memset(buffer, 0, sizeof(buffer));
                syncFieldByteErrorCount = 0;
                bitwiseSyncByte = SYNC_FIELD_1;

                // for (uint i = 0; i < MAX_BUFFER_BYTES; i++)
                // {
                //     if (buffer[i] == '\0' || buffer[i] == 6)
                //     {
                //         break;
                //     }
                //     else if (syncFieldMatch)
                //     {
                //         for (uint j = i; j < (i + MAX_SYNC_BYTES - 1); j++)
                //         {
                //             if (buffer[j] & bitwiseSyncByte)
                //                 bitwiseSyncByte++;
                //             else
                //             {
                //                 syncFieldByteErrorCount++;
                //                 j--;
                //             }
                            
                //             if (j == 16)
                //                 bitwiseSyncByte = SYNC_FIELD_2;

                //         }
                //         printf("Sync Field Error Byte Count: %d\n", syncFieldByteErrorCount);
                //         syncFieldByteErrorCount = 0;
                //     }
                //     else if (buffer[i] & bitwiseSyncByte)
                //     {
                //         syncFieldMatch = true;
                //         bitwiseSyncByte++;
                //     }
                //     else
                //     {
                //         for (uint j = 0; j < MAX_SYNC_BYTES - 1; j++)
                //         {
                //             if (j == 16)
                //             {

                //             }
                //         }
                //     }
                // }
            }

            fflush(stdout);
            mySwitch.resetAvailable();
        }

        usleep(100);
    }

    exit(0);
}
