#include <stdio.h>
#include <stdint.h>
#include "pack.h"
#include "varint.h"
#include "crc16.h"

int main(void){
    // Sample matches the Python example for interop testing
    plp_sample_t s = { .lat_deg=22.22, .lon_deg=34.43, .temp_c=56.0,
                       .battery_pct=87.5, .flags3=0b101, .time_s=40 };

    // Absolute pack/unpack roundtrip
    uint64_t word = plp_pack(&s);
    printf("Packed 64-bit word: %llu\n", (unsigned long long)word);

    plp_sample_t out;
    plp_unpack(word, &out);
    printf("Decoded: lat=%.2f lon=%.2f temp=%.1f batt=%.1f flags=%u time=%us\n",
        out.lat_deg, out.lon_deg, out.temp_c, out.battery_pct, out.flags3, out.time_s);

    // Delta + ZigZag + Varint mini-demo (just one field)
    int32_t dlat = 3;     // example small delta ticks
    uint32_t zz = zigzag32(dlat);
    uint8_t buf[8];
    size_t n = varint_encode32(zz, buf);
    printf("Delta %d -> ZigZag %u -> varint bytes %zu [", dlat, zz, n);
    for (size_t i=0;i<n;i++) printf("%02X%s", buf[i], i+1<n?" ":"");
    printf("]\n");

    uint32_t dec; size_t used = varint_decode32(buf, n, &dec);
    printf("Decoded varint: used=%zu value=%u -> unzigzag=%d\n", used, dec, unzigzag32(dec));

    // CRC-16 demo on the 64-bit word bytes (big-endian)
    uint8_t wbytes[8];
    for (int i=0;i<8;i++) wbytes[7-i] = (uint8_t)((word >> (i*8)) & 0xFF);
    uint16_t crc = crc16_ccitt(wbytes, 8, 0xFFFF);
    printf("CRC16-CCITT on packed word: 0x%04X\n", crc);

    return 0;
}
