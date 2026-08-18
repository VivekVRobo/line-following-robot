# Tooling

All tools use only the Python standard library.

- `simulate.py` — deterministic differential-drive reference simulation; regression/tuning aid, not a physical-performance claim.
- `calibrate_sensors.py` — converts labeled floor/line ADC samples into per-channel calibration arrays and polarity.
- `analyze_telemetry.py` — reports tracking/recovery ratios, line-error statistics, confidence and motor saturation.

Run tests with `python -m unittest discover -s tools/tests -v`.
