#include "util.h"

void memory_copy(char *source, char *dest, int no_bytes)
{
    int i;

    for (i = 0; i < no_bytes; i++)
    {
        dest[i] = source[i];
    }
}


unsigned char port_byte_in(unsigned short port)
{
    unsigned char result;

    __asm__ volatile (
        "in %%dx, %%al"
        : "=a" (result)
        : "d" (port)
    );

    return result;
}


void port_byte_out(unsigned short port, unsigned char data)
{
    __asm__ volatile (
        "out %%al, %%dx"
        :
        : "a" (data), "d" (port)
    );
}