from polarpack.encoder import pack_sample, unpack_sample
from polarpack.varint import zigzag_encode, zigzag_decode, varint_encode, varint_decode
from polarpack.crc16 import crc16_ccitt

def test_roundtrip():
    sample = (22.22, 34.43, 56.0, 87.5, 0b101, 40)
    word = pack_sample(*sample)
    out = unpack_sample(word)
    assert round(out["lat_deg"], 2) == 22.22
    assert round(out["lon_deg"], 2) == 34.43
    assert round(out["temp_c"], 1) == 56.0
    assert round(out["battery_pct"], 1) == 87.5
    assert out["flags3"] == 0b101
    assert out["time_s"] == 40

def test_varint():
    val = -123
    zz = zigzag_encode(val)
    v, used = varint_decode(varint_encode(zz))
    assert zigzag_decode(v) == val

def test_crc16():
    data = b"12345678"
    assert crc16_ccitt(data) == crc16_ccitt(data)
