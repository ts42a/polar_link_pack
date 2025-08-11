def zigzag_encode(v: int) -> int:
    return (v << 1) ^ (v >> 31)

def zigzag_decode(v: int) -> int:
    return (v >> 1) ^ -(v & 1)

def varint_encode(v: int) -> bytes:
    buf = bytearray()
    while v >= 0x80:
        buf.append((v & 0x7F) | 0x80)
        v >>= 7
    buf.append(v & 0x7F)
    return bytes(buf)

def varint_decode(b: bytes) -> tuple[int, int]:
    """Return (value, bytes_used)"""
    result = 0
    shift = 0
    for i, byte in enumerate(b):
        result |= (byte & 0x7F) << shift
        if not (byte & 0x80):
            return result, i + 1
        shift += 7
        if i >= 4:
            break
    raise ValueError("Invalid varint")
