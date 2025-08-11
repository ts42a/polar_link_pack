#ifndef PLP_VARINT_H
#define PLP_VARINT_H
#include <stdint.h>
#include <stddef.h>

// ZigZag maps signed to small unsigned for small magnitudes (matches Python).
static inline uint32_t zigzag32(int32_t v){ return (uint32_t)((v << 1) ^ (v >> 31)); }
static inline int32_t  unzigzag32(uint32_t v){ return (int32_t)((v >> 1) ^ (-(int32_t)(v & 1))); }

// Varint encode/decode (protobuf-style 7-bit groups).
size_t varint_encode32(uint32_t v, uint8_t *out);     // returns bytes written (1..5)
size_t varint_decode32(const uint8_t *in, size_t len, uint32_t *v_out); // bytes consumed or 0 on error

#endif
