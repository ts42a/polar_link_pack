#include "varint.h"

size_t varint_encode32(uint32_t v, uint8_t *out){
    size_t i = 0;
    while (v >= 0x80u){
        out[i++] = (uint8_t)((v & 0x7Fu) | 0x80u);
        v >>= 7;
    }
    out[i++] = (uint8_t)(v & 0x7Fu);
    return i;
}

size_t varint_decode32(const uint8_t *in, size_t len, uint32_t *v_out){
    uint32_t result = 0;
    int shift = 0;
    size_t i = 0;
    while (i < len && i < 5){
        uint8_t byte = in[i++];
        result |= ((uint32_t)(byte & 0x7Fu)) << shift;
        if ((byte & 0x80u) == 0){
            *v_out = result;
            return i;
        }
        shift += 7;
    }
    return 0; // error
}
