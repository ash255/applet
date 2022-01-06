#ifndef __BITS_H__
#define __BITS_H__
#include <stdint.h>

int bits_pack(uint8_t *bits, int len);
int bytes_to_bits(uint8_t *bits, int bits_len, uint8_t *bytes, int bytes_len);

#endif