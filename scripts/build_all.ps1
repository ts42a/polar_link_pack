@echo off
setlocal enabledelayedexpansion
echo [PLP] Building C...
make -C src\c
if %errorlevel% neq 0 exit /b %errorlevel%
echo [PLP] Installing Python package (editable)...
pip install -e src\python
if %errorlevel% neq 0 exit /b %errorlevel%
echo [PLP] Running tests...
pytest -q tests
if %errorlevel% neq 0 exit /b %errorlevel%
echo [PLP] Generating example data...
python examples\generate_samples.py --minutes 10 --rate-hz 2.0 --mode mild -o examples\_tmp.csv
if %errorlevel% neq 0 exit /b %errorlevel%
plp-encode examples\_tmp.csv -o %TEMP%\_plp.bin --keyframe-every 30
echo [PLP] Done.
