# PID & Sensor Tuning Guide

## 1. Validate sensors

Open the serial monitor and move the array across the track. Confirm the line produces a clearly different reading from the floor on every channel. Update `SENSOR_MIN`, `SENSOR_MAX`, and `SENSOR_DARK_LINE` to match your hardware.

## 2. Validate motors

Lift the wheels from the table and confirm a positive command rotates both wheels forward. Swap motor leads or invert direction logic if needed.

## 3. Tune steering

Use a low `BASE_SPEED` first.

1. Set `KI = 0` and `KD = 0`.
2. Increase `KP` until the robot follows the line but oscillates noticeably.
3. Increase `KD` until oscillation settles and corners remain responsive.
4. Add only a tiny `KI` if a persistent bias remains on long straight sections.
5. Increase `BASE_SPEED` gradually and repeat the process.

## 4. Diagnose behavior

| Symptom | Likely adjustment |
|---|---|
| Slow response / leaves corners | Increase `KP` |
| Rapid left-right oscillation | Reduce `KP` or increase `KD` |
| Twitchy response to noisy sensors | Increase derivative filtering or improve sensor calibration |
| Slowly drifts to one side | Check mechanics first; then consider tiny `KI` |
| Spins the wrong way when line is lost | Reverse `lastDirection` behavior or motor orientation |

Record final gains, track type, battery voltage, wheel diameter, and sensor height with test results so the repository reflects the actual robot configuration.
