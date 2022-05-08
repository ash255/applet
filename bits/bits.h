#ifndef __BITS_H__
#define __BITS_H__
#include <stdint.h>

typedef uint8_t ubit_t;

int bits_to_bytes(uint8_t *bytes, int bytes_len, ubit_t *bits, int bits_len);
int bits_pack(uint8_t *bits, int len);
int bytes_to_bits(ubit_t *bits, int bits_len, uint8_t *bytes, int bytes_len);

#endif
