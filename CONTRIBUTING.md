# Contributing

Contributions are welcome, especially improvements that keep hardware assumptions explicit and reproducible.

## Workflow

1. Fork the repository and create a focused branch.
2. Keep controller logic independent from board-specific I/O where possible.
3. Run `pio test -e native` for host tests.
4. Run `pio run -e uno` before submitting firmware changes.
5. Document any new pinout, sensor, motor driver, or calibration assumption.
6. Include measured results when claiming hardware performance.

Do not replace calibration placeholders with guessed values presented as verified measurements.
