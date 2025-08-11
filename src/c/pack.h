#ifndef PLP_PACK_H
#define PLP_PACK_H

#include <stdint.h>

typedef struct {
    double lat_deg;
    double lon_deg;
    double temp_c;
    double battery_pct;
    uint16_t flags3;   // lower 3 bits used
    uint32_t time_s;   // seconds since boot
} plp_sample_t;

// Pack to 64-bit word using the same scaling/bit layout as Python.
uint64_t plp_pack(const plp_sample_t *s);

// Unpack from 64-bit word to floating fields.
void     plp_unpack(uint64_t x, plp_sample_t *out);

#endif // PLP_PACK_H
