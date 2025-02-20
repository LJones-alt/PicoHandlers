#include "CRC16.h"

// Borrowed from https://stackoverflow.com/questions/22432066/how-to-use-table-based-crc-16-code

uint16_t // Returns Calculated CRC value
CRC16::Calculate(uint16_t crc, void *c_ptr, size_t len)
{
    const uint8_t *c = (uint8_t*) c_ptr;
    while (len--)
        crc = (crc << 8) ^ crctable[((crc >> 8) ^ *c++)];

    return crc;
}