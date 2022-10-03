//* Add in setup:   pinMode(OutputPin, OUTPUT);
//*                 digitalWrite(OutputPin, HIGH);

#include <Arduino.h>

// pulse parameters in usec
#define NEC_HDR_MARK 9000
#define NEC_HDR_SPACE 4500
#define NEC_BIT_MARK 560
#define NEC_ONE_SPACE 1600
#define NEC_ZERO_SPACE 560
#define NEC_RPT_SPACE 2250

#define TOPBIT 0x80000000

void mark(int pin, int time)
{
    digitalWrite(pin, LOW);
    delayMicroseconds(time);
}

void space(int pin, int time)
{
    digitalWrite(pin, HIGH);
    delayMicroseconds(time);
}

void sendRawHEX(int pin, unsigned long data, int nbits)
{
    mark(pin, NEC_HDR_MARK);
    space(pin, NEC_HDR_SPACE);

    for (int i = 0; i < nbits; i++)
    {
        if (data & TOPBIT)
        {
            mark(pin, NEC_BIT_MARK);
            space(pin, NEC_ONE_SPACE);
        }
        else
        {
            mark(pin, NEC_BIT_MARK);
            space(pin, NEC_ZERO_SPACE);
        }
        data <<= 1;
    }
    mark(pin, NEC_BIT_MARK);
    space(pin, 0);
}