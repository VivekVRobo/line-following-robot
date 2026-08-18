# Tuning

The tuning workflow has been expanded and split into focused documents:

- sensor/motor setup: [`CALIBRATION.md`](CALIBRATION.md)
- controller behavior: [`CONTROL_SYSTEM.md`](CONTROL_SYSTEM.md)
- objective physical testing: [`BENCHMARK_PROTOCOL.md`](BENCHMARK_PROTOCOL.md)

Use telemetry analysis instead of tuning only by visual impression:

```bash
python tools/analyze_telemetry.py serial.log --markdown
```
