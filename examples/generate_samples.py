#!/usr/bin/env python3
"""
Generate synthetic telemetry CSV streams for PLP.
"""
import csv, math, random, argparse

def make_stream(minutes=10, rate_hz=2.0, mode="mild"):
    dt = 1.0 / rate_hz
    t = 0.0
    lat, lon = 80.0, 0.0  # high-lat start
    temp = -15.0
    bat = 100.0
    flags = 0b101
    rows = []
    steps = int(minutes * 60 * rate_hz)
    for k in range(steps):
        # motion model
        if mode == "mild":
            lat += 0.0008 + 0.0002*math.sin(0.01*k)
            lon += 0.0010 + 0.0003*math.cos(0.013*k)
        elif mode == "turns":
            lat += 0.0006 + 0.0005*math.sin(0.05*k)
            lon += 0.0011 + 0.0010*math.sin(0.033*k)
        elif mode == "sparse":
            if k % int(rate_hz*15) == 0:
                lat += 0.02
                lon += 0.02
        else:
            lat += 0.001
            lon += 0.001

        # environment
        if k % 200 == 0:
            temp += random.choice([0.0, -0.1, 0.1])
        bat -= 0.005  # slow drain

        rows.append([lat, lon, temp, max(0.0, bat), flags, int(t)])
        t += dt
    return rows

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--minutes", type=float, default=30.0)
    ap.add_argument("--rate-hz", type=float, default=2.0)
    ap.add_argument("--mode", choices=["mild","turns","sparse"], default="mild")
    ap.add_argument("-o","--out", default="generated.csv")
    args = ap.parse_args()

    rows = make_stream(args.minutes, args.rate_hz, args.mode)
    with open(args.out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["lat","lon","temp_c","battery_pct","flags3","time_s"])
        w.writerows(rows)
    print(f"Wrote {len(rows)} rows to {args.out}")

if __name__ == "__main__":
    main()
