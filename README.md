# Line Following Robot

A configurable Arduino line-following robot reference implementation with weighted sensor error, PID steering, motor mixing, lost-line recovery, and a practical tuning workflow.

> **Project status:** reference implementation ready for hardware integration. Pin assignments, sensor polarity, motor direction, and PID gains must be calibrated for the actual robot.

## Highlights

- 5-sensor weighted line position estimator
- PID steering with integral clamping and derivative filtering
- Differential-drive motor mixer
- Lost-line recovery using the last known direction
- Serial telemetry for tuning
- PlatformIO project for reproducible builds

## Suggested hardware

- Arduino Uno/Nano compatible board
- 5-channel IR reflectance sensor array
- TB6612FNG or compatible dual H-bridge
- 2 geared DC motors + wheels
- Separate motor supply with a common ground

## Default pin map

| Function | Pin |
|---|---:|
| Sensors left → right | A0, A1, A2, A3, A4 |
| Left motor PWM | 5 |
| Left motor direction | 7, 8 |
| Right motor PWM | 6 |
| Right motor direction | 9, 10 |

## Build

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

## Control loop

The five sensors are converted to line strengths and assigned positions `-2000, -1000, 0, 1000, 2000`. Their weighted average estimates lateral line error. A PID controller calculates a steering correction and the motor mixer applies:

```text
left  = baseSpeed + correction
right = baseSpeed - correction
```

When the line disappears, the robot rotates toward the last observed line direction instead of continuing blindly.

## Calibration checklist

1. Confirm all sensor values increase (or decrease) consistently over the line.
2. Set `SENSOR_DARK_LINE` in `src/main.cpp` for black-line vs white-line operation.
3. Measure floor and line readings and adjust `SENSOR_MIN` / `SENSOR_MAX`.
4. Verify each motor's forward direction.
5. Start with `KI = 0`, tune `KP`, then `KD`, and add a very small `KI` only if necessary.
6. Raise `BASE_SPEED` after stable tracking is achieved.

See [`docs/TUNING.md`](docs/TUNING.md) for a structured tuning procedure.

## Safety

Test with the wheels lifted first. Do not power motors from the microcontroller's 5 V pin. Use a suitable motor supply and common ground.

## Roadmap

- [x] Weighted 5-sensor position estimation
- [x] PID controller
- [x] Lost-line recovery
- [x] Serial telemetry
- [ ] Hardware calibration values
- [ ] Track test results / plots
- [ ] Optional encoder-based speed regulation

## License

MIT-style usage is intended; add a formal license file before redistribution if required.
