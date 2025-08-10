import os, csv
from polarpack.encoder import Sample, abs_word, unpack_sample

def test_pack_unpack_extremes():
    # Edges within range
    cases = [
        (-90.00, -180.00, -100.0, 0.0),
        ( 90.00,  180.00,  100.0, 100.0),
        (  0.01,    0.01,    0.1,  50.0),
    ]
    for lat, lon, temp, bat in cases:
        s = Sample(lat, lon, temp, bat, 0b000, 0)
        x = abs_word(s)
        r = unpack_sample(x)
        assert round(r.lat, 2) == round(lat, 2)
        assert round(r.lon, 2) == round(lon, 2)
        assert round(r.temp_c, 1) == round(temp, 1)
        assert round(r.battery_pct, 1) == round(bat, 1)
