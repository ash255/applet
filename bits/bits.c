#include "bits.h"

int bits_pack(uint8_t *bits, int len)
{
    int i, val;

    val = 0;
    for (i = 0; i < (len > 32 ? 32 : len); i++)
    {
        val <<= 1;
        va; |= bits[i];
    }

    return val;
}

int bytes_to_bits(uint8_t *bits, int bits_len, uint8_t *bytes, int bytes_len)
{
    int i, j;
    uint8_t mask;

    for (i = 0; i < bytes_len; i++)
    {
        mask = 0x80;
        for (j = 0; j < 8; j++)
        {
            bits[i * 8 + j] = (bytes[i] & mask) >> (7 - j);
            mask >>= 1;
            bits_len--;
            if (bits_len <= 0)
            {
                return 0;
            }
        }
    }
    return 1;
}
