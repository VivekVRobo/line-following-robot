# Simulation and Regression

The simulator is a deterministic software reference model used to catch controller regressions before hardware testing. It is intentionally simple and must not be used as evidence of real-world lap time, stability, safety, or accuracy.

## Single run

```bash
python tools/simulate.py --seconds 20
```

Outputs include cross-track error, robot pose, wheel commands and a JSON summary.

## Regression gate

```bash
python tools/regression_suite.py --seconds 8
```

The suite evaluates 20 scenarios spanning four random seeds and five initial lateral offsets. CI fails when the aggregate reference model exceeds configured limits for mean RMS cross-track error, worst-case RMS error, recovery ratio, or forward progress.

These thresholds are **software regression thresholds**. They are deliberately separated from the physical benchmark protocol in `BENCHMARK_PROTOCOL.md`.

## Why this exists

A controller can compile and still become materially worse. The regression suite provides a repeatable behavioral check so future changes to PID, speed planning, mixing, or recovery can be compared against a stable reference.
