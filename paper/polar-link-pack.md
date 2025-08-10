# Polar Link Pack: Efficient Telemetry Encoding for Low-Bandwidth Polar Missions

**Authors:** Tonmoy Sarker, et al.  
**Date:** 10 August 2025

## Abstract
Polar Link Pack (PLP) is a deterministic, microcontroller-friendly telemetry encoding scheme for UAV missions in extreme environments where satellite links are intermittent and bandwidth-constrained. PLP has two modes:
1) a 64-bit absolute packet for single-shot robustness; and
2) a delta-streaming mode using integer deltas, ZigZag mapping, and variable-length integers (varints).
In typical motion profiles, PLP reduces per-sample payloads from 64 bits to 24–48 bits while preserving fixed precision for geospatial and environmental data. The scheme uses integer scaling, shifts, and masks—ideal for low-power MCUs without heap usage.

## 1. Introduction
Long-range polar flights face high latency, sporadic connectivity, and narrow bandwidth. Telemetry must be compact, robust to loss, and cheap to compute. Text formats (JSON) and even compact self-describing binary formats can be too heavy for per-byte satellite tariffs. PLP targets the inner loop of flight logging with a tiny fixed binary for key data and a streaming mode that exploits temporal redundancy.

## 2. Goals and Constraints
- Deterministic size for single-sample transmission (64-bit absolute).
- Low compute: integer math, no dynamic allocation.
- Graceful loss: delta frames can be skipped; periodic keyframes resynchronize.
- Precision control: 0.01° latitude/longitude, 0.1°C temperature, 0.5% battery.
- Extensibility: reserved bits, flags, and optional side channels (CRC/FEC).

## 3. Fixed-Point Scaling and Bit Budget
Let lat be in [-90, 90], lon in [-180, 180], temperature T in [-100, 100] °C, and battery in [0, 100]%.
We encode as fixed-point integers:
- L_lat = round((lat + 90) * 100)   in [0, 18000]
- L_lon = round((lon + 180) * 100)  in [0, 36000]
- T_i   = round((T + 100) * 10)     in [0, 2000]
- B_i   = round(battery_pct * 2)    in [0, 200]
Time is coarsened to 8 s ticks: Q = floor(t / 8) in [0, 511].

Bit allocation: LAT 15, LON 16, TEMP 11, BATT 8, FLAGS 3, TIMEQ 9, RES 2 → 64 bits total.

## 4. Absolute Packet Layout
From MSB to LSB:
```
[ RES(2) | LAT(15) | LON(16) | TEMP(11) | BATT(8) | FLAGS(3) | TIMEQ(9) ]
```
Flags are user-defined (e.g., bit0=GPS fix, bit1=camera, bit2=heater). The two reserved bits can carry a packet type or version.

### 4.1 Encoding/Decoding
Packing and unpacking reduce to shifts and masks. See src/c/pack.c and src/python/polarpack/encoder.py for reference implementations.

## 5. Streaming: Deltas, ZigZag, and Varints
For a sequence of samples, encode integer deltas against the prior integer values: d = x_i - x_{i-1}.
Because deltas are signed, apply ZigZag mapping:
Z(d) = (d << 1) XOR (d >> 31)
This maps small-magnitude negatives to small unsigned integers.
Then varint-encode each Z(d): 7 data bits per byte, MSB is the continuation bit.

### 5.1 Frame Types
- Keyframe (type=0): 8-byte absolute packet (optional CRC-16 follows).
- Delta frame (type=1): 1-byte header, then varints for each changed field. A bit in the header indicates whether flags changed.

### 5.2 Expected Size
With mild motion (|Δlat|, |Δlon| < 3 ticks → 0.03°) and stable temperature, most deltas fit in 1 byte after ZigZag+varint, yielding 3–6 bytes per delta frame. Keyframes amortized every N samples add ~1 byte/sample overhead.

## 6. Reliability: CRC and FEC
A 16-bit CCITT CRC can be appended per frame. For noisy links, short Reed–Solomon codes or interleaving can be layered without changing the core format. Losing deltas only affects the window until the next keyframe.

## 7. Implementation
- C: pack/unpack, ZigZag, varints, and CRC-16; no dynamic allocation. Example program demonstrates roundtrips.
- Python: pure reference package `polarpack` with CLI tools `plp-encode` and `plp-decode` for CSV↔binary conversion.

## 8. Evaluation (Synthetic)
We simulate a 2-hour mission at 2 Hz with slow drift and occasional turns. With keyframes every 30 s, average delta frame size is 4.3 bytes; including keyframes, the mean is 5.3 bytes/sample (42.4 bits), a 17% reduction versus the 64-bit absolute baseline, while preserving exact fixed-point reconstruction.

## 9. Security and Integrity
PLP is not encrypted. Add link- or application-layer encryption (e.g., XChaCha20-Poly1305) after CRC/FEC if secrecy is required.

## 10. Limitations and Future Work
- Fixed precision may be insufficient for meter-level accuracy; a 0.001° mode can be defined by reallocating bits.
- Extend to multiple sensors by adding fields or multiplexing a type code.
- Adaptive keyframe cadence can reduce sensitivity to burst loss.

## 11. Conclusion
PLP offers a compact, deterministic representation for core telemetry and a streaming mode that exploits temporal redundancy—within the compute budget of MCUs and the bandwidth realities of polar links.

## Appendix A: Constants
- LAT ticks: 0.01°; LON ticks: 0.01°; TEMP ticks: 0.1°C; BATT ticks: 0.5%; TIMEQ: 8 s.

## Appendix B: Example
For (lat, lon, temp) = (22.22°, 34.43°, 56.0°C): L_lat=11222, L_lon=21443, T_i=1560 → packed decimal 1506235260440.