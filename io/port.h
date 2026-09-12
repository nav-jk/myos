#ifndef PORT_H
#define PORT_H

#include <stdint.h>

/* Read/write 8 bits */
uint8_t port_byte_in(uint16_t port);
void port_byte_out(uint16_t port, uint8_t data);

/* Read/write 16 bits */
uint16_t port_word_in(uint16_t port);
void port_word_out(uint16_t port, uint16_t data);

#endif