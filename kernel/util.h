#ifndef UTIL_H
#define UTIL_H

void memory_copy(char *source, char *dest, int no_bytes);

unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);

#endif