# PLP ASCII Diagrams

These ASCII diagrams mirror the LaTeX figures in `paper/polar-link-pack.tex` so they render nicely on GitHub too.

## 64-bit Packet Bitfield Layout (MSB→LSB)
```
Bit index: 63                                                   0
           +--RES--+---------------LAT---------------+------LON------+----TEMP----+--BATT--+-FLG-+--TIMEQ--+
Width:       [2]                   [15]                     [16]           [11]       [8]   [3]     [9]

Field:       RES                 LAT (0.01°)             LON (0.01°)     TEMP (0.1°C) BATT(0.5%) FLAGS  TIMEQ(8s)
Bits:      63..62               61..47                   46..31          30..20       19..12   11..9   8..0
Example:   00     | 010101001010010 | 1000011100100011 | 10100010101 | 10101110 | 101 | 001000100
```

## Delta Frame Application Pipeline
```
          +------------------+        +-------------------+        +---------------------+
Prev abs  |  L_lat, L_lon,   |  Δ     |  ZigZag each Δ    | varint |  Decode varints     |  Δ^-1   New abs
state --->|  T_i, B_i, Q, f  | -----> |  Z(d)=(d<<1)^(d>>31) ----->|  -> unsigned ints   | ----->  state
          +------------------+        +-------------------+        +---------------------+

Header bits in delta frame specify which fields are included:
   [LAT][LON][TEMP][BATT][FLAGS][TIME]  (1 = field present, 0 = skip)
```

## ZigZag Mapping Intuition
```
Signed d:   0   -1   +1   -2   +2   -3   +3   -4   +4
ZigZag Z(d):0    1    2    3    4    5    6    7    8   ...

Property: small-magnitude signed integers -> small unsigned integers.
This improves varint packing because small unsigned values need fewer bytes.
```

## Varint Continuation Semantics (7 data bits / byte)
```
Example value: 0x0000012C = 300 decimal
Binary: 0000 0000 0000 0000 0000 0001 0010 1100

Chunk into 7-bit groups (LSB-first):
   [0101100] [0000010]

Emit bytes least-significant group first; set MSB=1 on all but last:
   Byte0: 1010 1100  = 0xAC   (0x80 | 0x2C)
   Byte1: 0000 0010  = 0x02   (last byte, MSB=0)

Decoder reads until a byte with MSB=0 is seen.
```
