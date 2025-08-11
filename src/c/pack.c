#include "pack.h"
#include <math.h>

static inline uint32_t clamp_u32(uint32_t v, uint32_t lo, uint32_t hi){
    if(v < lo) return lo;
    if(v > hi) return hi;
    return v;
}

uint64_t plp_pack(const plp_sample_t *s) {
    // Fixed-point scaling (matches Python encoder.py)
    uint32_t lat_i  = (uint32_t) llround((s->lat_deg  + 90.0)  * 100.0);   // 15b
    uint32_t lon_i  = (uint32_t) llround((s->lon_deg  + 180.0) * 100.0);   // 16b
    uint32_t tmp_i  = (uint32_t) llround((s->temp_c   + 100.0) * 10.0);    // 11b
    uint32_t bat_i  = (uint32_t) llround((s->battery_pct * 2.0));          // 8b
    uint32_t time_q = (uint32_t)((s->time_s) / 8u) & 0x1FFu;               // 9b

    // Clamp to defined ranges
    lat_i = clamp_u32(lat_i, 0, 18000);
    lon_i = clamp_u32(lon_i, 0, 36000);
    tmp_i = clamp_u32(tmp_i, 0, 2000);
    bat_i = clamp_u32(bat_i, 0, 200);

    // Bit layout (MSB → LSB):
    // [ RES(2) | LAT(15) | LON(16) | TEMP(11) | BATT(8) | FLAGS(3) | TIMEQ(9) ] = 64 bits
    uint64_t x = 0;
    x |= ((uint64_t)lat_i  & 0x7FFFULL) << 47; // 15
    x |= ((uint64_t)lon_i  & 0xFFFFULL) << 31; // 16
    x |= ((uint64_t)tmp_i  & 0x7FFULL)  << 20; // 11
    x |= ((uint64_t)bat_i  & 0xFFULL)   << 12; // 8
    x |= ((uint64_t)(s->flags3 & 0x7U)) << 9;  // 3
    x |= ((uint64_t)time_q & 0x1FFULL);        // 9
    return x;
}

void plp_unpack(uint64_t x, plp_sample_t *out) {
    uint32_t time_q =  x         & 0x1FFu;
    uint32_t flags  = (x >> 9)   & 0x7u;
    uint32_t bat_i  = (x >> 12)  & 0xFFu;
    uint32_t tmp_i  = (x >> 20)  & 0x7FFu;
    uint32_t lon_i  = (x >> 31)  & 0xFFFFu;
    uint32_t lat_i  = (x >> 47)  & 0x7FFFu;

    out->lat_deg     = ((double)lat_i / 100.0) - 90.0;
    out->lon_deg     = ((double)lon_i / 100.0) - 180.0;
    out->temp_c      = ((double)tmp_i / 10.0)  - 100.0;
    out->battery_pct = ((double)bat_i / 2.0);
    out->flags3      = (uint16_t)flags;
    out->time_s      = (uint32_t)(time_q * 8u);
}
