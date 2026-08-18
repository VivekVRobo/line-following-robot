# Changelog

## Unreleased

### Added
- confidence-aware per-sensor line estimator
- adaptive speed planner
- staged recovery planner
- saturation-preserving motor mixer
- safe-stop serial command interface
- deterministic simulator
- 20-scenario simulation regression gate
- sensor calibration CLI
- telemetry analysis CLI
- expanded native/Python tests and CI
- benchmark, simulation, safety, calibration and troubleshooting documentation

### Changed
- PID includes conditional-integration anti-windup and output limiting
- calibration is per sensor with explicit raw polarity
- firmware boots stopped until `START`
- telemetry uses a documented CSV schema
