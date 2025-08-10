#!/usr/bin/env bash
set -euo pipefail
echo "[PLP] Building C..."
make -C src/c
echo "[PLP] Installing Python package (editable)..."
pip install -e src/python
echo "[PLP] Running tests..."
pytest -q tests
echo "[PLP] Generating examples and figure..."
python examples/generate_samples.py --minutes 10 --rate-hz 2.0 --mode mild -o examples/_tmp.csv
plp-encode examples/_tmp.csv -o /tmp/_plp.bin --keyframe-every 30 || true
echo "[PLP] Done."
