# Line Following Robot

[![PlatformIO CI](https://github.com/vasu4990/line-following-robot/actions/workflows/platformio.yml/badge.svg)](https://github.com/vasu4990/line-following-robot/actions/workflows/platformio.yml)

A production-style Arduino reference implementation for a differential-drive line-following robot. The controller combines a five-sensor weighted line estimator, PID steering, motor mixing, lost-line recovery, and serial telemetry.

> **Status:** software/reference design complete; physical calibration and track validation must be performed on the target robot.

## Features

- 5-channel analog reflectance sensor support
- Weighted line-position estimation
- PID steering with derivative filtering and integral clamping
- Differential-drive motor mixing
- Lost-line recovery using the last known direction
- Configurable black-line / white-line sensing
- Serial telemetry for tuning and diagnostics
- PlatformIO builds for Arduino Uno and native unit tests
- GitHub Actions CI

## Repository layout

```text
.
├── .github/workflows/platformio.yml
├── docs/
│   ├── ARCHITECTURE.md
│   ├── HARDWARE.md
│   └── TUNING.md
├── include/
│   ├── config.h
│   └── control.h
├── src/main.cpp
├── test/test_control/test_main.cpp
├── platformio.ini
└── README.md
```

## Hardware reference

The default configuration targets an Arduino Uno/Nano-class board, a 5-channel analog IR sensor array, a TB6612FNG/L298N-style dual motor driver, and two geared DC motors. See [`docs/HARDWARE.md`](docs/HARDWARE.md) before wiring.

### Default pin map

| Function | Pin |
|---|---:|
| Sensors left → right | A0, A1, A2, A3, A4 |
| Left motor PWM | 5 |
| Left direction | 7, 8 |
| Right motor PWM | 6 |
| Right direction | 9, 10 |

## Build and upload

Install [PlatformIO](https://platformio.org/) and run:

```bash
pio run -e uno
pio run -e uno -t upload
pio device monitor -b 115200
```

Run host-side unit tests:

```bash
pio test -e native
```

## Calibration

All hardware-dependent values are centralized in [`include/config.h`](include/config.h). Before driving on a track:

1. Verify motor direction with the wheels lifted.
2. Measure raw sensor values over the floor and line.
3. Update sensor min/max calibration values.
4. Select dark-line or light-line mode.
5. Tune `KP`, then `KD`, and only then add a small `KI` if needed.
6. Increase base speed only after stable tracking.

See [`docs/TUNING.md`](docs/TUNING.md) for the tuning procedure.

## Control model

Each sensor is mapped to a normalized strength and assigned a lateral coordinate. The weighted average estimates line position. The PID output becomes a steering correction:

```text
leftPWM  = baseSpeed + correction
rightPWM = baseSpeed - correction
```

If no sensor sees the line, the robot performs a controlled search toward the last known line direction.

## Safety

- Never power DC motors from the Arduino 5 V pin.
- Use a motor supply appropriate for the motors and driver.
- Connect controller and motor-driver grounds together.
- Test direction and emergency stop behavior with wheels off the ground first.
- Treat the default gains and thresholds as starting values, not verified hardware values.

## Roadmap

- [x] Five-sensor weighted estimator
- [x] PID control
- [x] Lost-line recovery
- [x] Host-side controller tests
- [x] CI build/test workflow
- [ ] Robot-specific sensor calibration
- [ ] Track benchmark data
- [ ] Optional wheel-encoder speed loop

## Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md).

## License

MIT — see [`LICENSE`](LICENSE).
