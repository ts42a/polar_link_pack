#ifndef PLP_CRC16_H
#define PLP_CRC16_H
#include <stdint.h>

// CCITT CRC-16 (poly 0x1021), conventional init.
uint16_t crc16_ccitt(const uint8_t *data, int len, uint16_t init);

#endif
