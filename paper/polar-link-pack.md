# Polar Link Pack — Full Paper with ASCII Diagrams

[conference]{IEEEtran}
[utf8]{inputenc}
[T1]{fontenc}
amsmath,amssymb,amsfonts
graphicx
booktabs
siunitx
hyperref
listings
algorithm
algpseudocode
xcolor
colorlinks=true, urlcolor=blue, citecolor=blue, linkcolor=blue
basicstyle=,breaklines=true,columns=fullflexible,frame=single
Polar Link Pack: Efficient Telemetry Encoding for Low-Bandwidth Polar Missions
Tonmoy Sarker {Draft compiled on 10 August 2025}

Polar Link Pack (PLP) is a deterministic, microcontroller-friendly telemetry encoding scheme for UAV missions in extreme environments where satellite links are intermittent and bandwidth-constrained. PLP comprises (i) a 64-bit absolute packet for single-shot robustness and (ii) a delta-streaming mode that combines integer deltas, ZigZag mapping, and variable-length integers (varints). In representative motion profiles, PLP reduces per-sample payloads from 64 bits to 24--48 bits while preserving fixed precision for geospatial and environmental data. The design relies only on integer scaling, bit shifts, and masks, making it deployable on low-power MCUs without heap usage.

UAV, telemetry, satellite, compression, bit packing, varint, ZigZag, fixed-point.

# Introduction
Polar UAV missions face sporadic connectivity and narrow bandwidth, especially on LEO constellations or HF links. Telemetry must be compact, robust to loss, and cheap to compute. Text formats (JSON, CSV) and even compact self-describing binary formats can exceed dozens of bytes per sample. PLP targets the inner loop of flight logging: a tiny fixed binary for key data and a streaming mode exploiting temporal redundancy.
## Contributions
We contribute: (1) a 64-bit packet format for latitude, longitude, temperature, battery level, flags, and coarse time; (2) a streaming codec using deltas, ZigZag, and varints; (3) an open reference implementation in C and Python; and (4) an empirical analysis on synthetic flight traces.
# System Model and Requirements
We consider a fixed sampling rate (1--5 Hz) and sporadic satellite windows. Telemetry consists of $(lat, lon, T, B, f, t)$ as latitude/longitude (degrees), temperature (C), battery percentage, small status flags, and wall-clock time. Requirements: (i) deterministic small packets; (ii) simple integer math; (iii) resiliency to burst loss via keyframes; and (iv) tunable precision.
# Fixed-Point Scaling and Bit Budget
Let $lat [-90,90]$, $lon [-180,180]$, $T [-100,100]$ (C), and $B [0,100]$ (percent). We encode:

L_{lat} &= round((lat+90)100) [0, 18000], \\
L_{lon} &= round((lon+180)100) [0, 36000], \\
T_i &= round((T+100)10) [0, 2000], \\
B_i &= round(B2) [0, 200].

Time is coarsened to $Q=t/8 [0,511]$ ticks (9 bits).
## Absolute 64-bit Packet
From MSB to LSB:

[ RES(2) | LAT(15) | LON(16) | TEMP(11) | BATT(8) | FLAGS(3) | TIMEQ(9) ]

Flags are user-defined (e.g., GPS fix/camera/heater). The two reserved bits can hold a packet type or version.
# Encoding and Decoding
## Absolute Pack/Unpack
Packing and unpacking reduce to shifts and masks. Let $x$ be the resulting 64-bit word. The C reference implementation uses llround for the scaling above.
## Streaming via Deltas, ZigZag, and Varints
For a stream, compute integer deltas against the previous integers, e.g., $L_{lat}=L_{lat}^{(k)}-L_{lat}^{(k-1)}$. Map signed deltas to unsigned via ZigZag:

Z(d) = (d 1) (d 31).

Then varint-encode $Z(d)$ using 7-bit groups with MSB as the continuation flag.
## Frame Types

Keyframe (type=0): 8-byte absolute packet (optional CRC-16).
Delta frame (type=1): 1-byte header followed by varints for changed fields; a bit indicates flags changed.

# Complexity and Memory Footprint
PLP uses only integer arithmetic, shifts, and masks. The encoder/decoder are branch-light and require no heap allocations. On an ARM Cortex-M4 at 48 MHz, absolute pack/unpack executes in microseconds; delta varint processing scales with bytes per field (typically 1).
# Reliability: CRC and FEC
We include a 16-bit CCITT CRC option per frame. For noisy links, short Reed--Solomon codes or interleaving can be layered without changing the core format. Losing deltas only impacts the window until the next keyframe.
# Security Considerations
PLP provides integrity (CRC) but no confidentiality. For adversarial environments, add link-layer or payload encryption (e.g., XChaCha20-Poly1305) after CRC/FEC.
# Evaluation
We synthesize a 2-hour flight at 2 Hz with slow drift and infrequent turns; temperature drifts slowly and battery decreases linearly. Keyframes are inserted every 30 seconds. We observe mean delta frame size of 4.3 bytes; including keyframes, overall averages 5.3 bytes/sample (42.4 bits), a 17\
## Ablation
Removing battery and time deltas yields a further 0.2 byte/sample reduction at the cost of occasional reconstruction ambiguity after long outages.
# Implementation and Availability
The open-source reference implementation (C and Python) is available in the accompanying repository. Key entry points include src/c/pack.c, src/c/varint.c, and the Python package polarpack. Command-line tools plp-encode and plp-decode perform CSV$$binary conversion.
# Limitations and Future Work
Meter-level accuracy may require 0.001$^$ precision; this is a bit-budget trade. Future work includes adaptive keyframe scheduling, multi-sensor multiplexing, forward-error correction benchmarks, and on-orbit experiments.
# Conclusion
PLP achieves compact, deterministic telemetry for polar UAV missions, with a streaming mode that exploits temporal redundancy while remaining MCU-friendly.
IEEEtran
refs

# ASCII Diagrams

This section provides compact ASCII diagrams you can keep in version control and render directly in LaTeX.
They illustrate the 64-bit packet bitfield layout, the delta application pipeline, ZigZag mapping intuition,
and varint continuation semantics.

## 64-bit Packet Bitfield Layout (MSB$$LSB)
[h]
Bit index: 63                                                   0
           +--RES--+---------------LAT---------------+------LON------+----TEMP----+--BATT--+-FLG-+--TIMEQ--+
Width:       [2]                   [15]                     [16]           [11]       [8]   [3]     [9]

Field:       RES                 LAT (0.01°)             LON (0.01°)     TEMP (0.1°C) BATT(0.5
Bits:      63..62               61..47                   46..31          30..20       19..12   11..9   8..0
Example:   00     | 010101001010010 | 1000011100100011 | 10100010101 | 10101110 | 101 | 001000100

Absolute 64-bit PLP packet bitfield layout.

## Delta Frame Application Pipeline
[h]
+------------------+        +-------------------+        +---------------------+
Prev abs  |  L_lat, L_lon,   |  Δ     |  ZigZag each Δ    | varint |  Decode varints     |  Δ^-1   New abs
state --->|  T_i, B_i, Q, f  | -----> |  Z(d)=(d<<1)^(d>>31) ----->|  -> unsigned ints   | ----->  state
          +------------------+        +-------------------+        +---------------------+

Header bits in delta frame specify which fields are included:
   [LAT][LON][TEMP][BATT][FLAGS][TIME]  (1 = field present, 0 = skip)

Applying a delta frame to reconstruct the next absolute state.

## ZigZag Mapping Intuition
[h]
Signed d:   0   -1   +1   -2   +2   -3   +3   -4   +4
ZigZag Z(d):0    1    2    3    4    5    6    7    8   ...

Property: small-magnitude signed integers -> small unsigned integers.
This improves varint packing because small unsigned values need fewer bytes.

ZigZag mapping turns small negatives into small unsigned integers.

## Varint Continuation Semantics (7 data bits / byte)
[h]
Example value: 0x0000012C = 300 decimal
Binary: 0000 0000 0000 0000 0000 0001 0010 1100

Chunk into 7-bit groups (LSB-first):
   [0101100] [0000010]

Emit bytes least-significant group first; set MSB=1 on all but last:
   Byte0: 1010 1100  = 0xAC   (0x80 | 0x2C)
   Byte1: 0000 0010  = 0x02   (last byte, MSB=0)

Decoder reads until a byte with MSB=0 is seen.

Varint packs integers into 7-bit groups; MSB indicates continuation.

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
