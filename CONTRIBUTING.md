# Contributing

Contributions are welcome when they improve correctness, testability, documentation, or reproducibility.

## Local checks
```bash
pio run -e uno
pio test -e native
python -m unittest discover -s tools/tests -v
```

## Control changes
Add/update native tests for PID, estimator, mixer, speed planner, parser, or recovery behavior. If tuning is based on physical hardware, include track conditions and raw telemetry.

## Hardware changes
Document pin map, polarity, voltage/current, sensor height and driver assumptions. Do not present an unbuilt or unmeasured configuration as validated.

## Performance claims
Follow `docs/BENCHMARK_PROTOCOL.md`. Simulator output must be labeled simulation. Physical results should include raw logs and exact commit/configuration.

## Pull requests
Keep changes focused, explain safety impact, and ensure CI passes before review.
