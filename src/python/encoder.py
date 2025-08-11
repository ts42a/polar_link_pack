import struct

def pack_sample(lat_deg, lon_deg, temp_c, battery_pct, flags3, time_s):
    """
    Pack a sample into a 64-bit integer.
    Scaling and bit layout exactly match C version in pack.c.
    """
    lat_i  = int(round((lat_deg + 90.0)  * 100.0))     # 15b
    lon_i  = int(round((lon_deg + 180.0) * 100.0))     # 16b
    tmp_i  = int(round((temp_c + 100.0)  * 10.0))      # 11b
    bat_i  = int(round(battery_pct * 2.0))             # 8b
    time_q = (time_s // 8) & 0x1FF                     # 9b

    # clamp ranges
    lat_i = max(0, min(lat_i, 18000))
    lon_i = max(0, min(lon_i, 36000))
    tmp_i = max(0, min(tmp_i, 2000))
    bat_i = max(0, min(bat_i, 200))

    x = 0
    x |= (lat_i & 0x7FFF)  << 47
    x |= (lon_i & 0xFFFF)  << 31
    x |= (tmp_i & 0x7FF)   << 20
    x |= (bat_i & 0xFF)    << 12
    x |= (flags3 & 0x7)    << 9
    x |= (time_q & 0x1FF)

    return x

def unpack_sample(word):
    """
    Unpack a 64-bit integer into fields.
    """
    time_q =   word        & 0x1FF
    flags3 =  (word >> 9)  & 0x7
    bat_i  =  (word >> 12) & 0xFF
    tmp_i  =  (word >> 20) & 0x7FF
    lon_i  =  (word >> 31) & 0xFFFF
    lat_i  =  (word >> 47) & 0x7FFF

    lat_deg     = (lat_i / 100.0) - 90.0
    lon_deg     = (lon_i / 100.0) - 180.0
    temp_c      = (tmp_i / 10.0) - 100.0
    battery_pct = bat_i / 2.0
    time_s      = time_q * 8

    return {
        "lat_deg": lat_deg,
        "lon_deg": lon_deg,
        "temp_c": temp_c,
        "battery_pct": battery_pct,
        "flags3": flags3,
        "time_s": time_s
    }

def pack_bytes(*args):
    """Pack sample → big-endian bytes."""
    word = pack_sample(*args)
    return word.to_bytes(8, "big")

def unpack_bytes(b):
    """Unpack from big-endian bytes."""
    word = int.from_bytes(b, "big")
    return unpack_sample(word)
