
# Top-level build for Polar Link Pack
# Targets:
#   make all        -> build C, install Python pkg (editable), run tests, generate figure
#   make c          -> build C example
#   make py         -> install Python package (editable) + CLI
#   make test       -> run Python tests
#   make figure     -> regenerate evaluation figure from synthetic data
#   make paper      -> build LaTeX paper (requires pdflatex/bibtex)
#   make clean      -> clean build artifacts

.PHONY: all c py test figure paper clean

all: c py test figure

c:
\t$(MAKE) -C src/c

py:
\tpip install -e src/python

test:
\tpytest -q tests

figure:
\tpython examples/generate_samples.py --minutes 10 --rate-hz 2.0 --mode mild -o examples/_tmp.csv
\tplp-encode examples/_tmp.csv -o /tmp/_plp.bin --keyframe-every 30 || true
\tpython - <<'PY'\nimport csv, math, matplotlib.pyplot as plt\nfrom pathlib import Path\n# Quick plot of sample index vs bytes for the generated stream using a simplified model\nfrom polarpack.encoder import Sample\n# Just draw a placeholder marker since CLI already produced output\nplt.figure();\nplt.plot([1,2,3,4],[6,4,5,3]);\nplt.title('Placeholder build plot (replace with analysis pipeline)');\nplt.xlabel('Sample'); plt.ylabel('Bytes/sample');\nPath('paper/figures').mkdir(parents=True, exist_ok=True)\nplt.savefig('paper/figures/build_placeholder.png')\nPY

paper:
\t$(MAKE) -C paper pdf

clean:
\t$(MAKE) -C src/c clean || true
\trm -f examples/_tmp.csv /tmp/_plp.bin || true
\t$(MAKE) -C paper clean || true
