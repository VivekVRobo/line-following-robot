# PID and Sensor Tuning

## 1. Sensor calibration

Open the serial monitor and temporarily print raw `analogRead()` values if necessary. Record minimum and maximum values over both floor and line. Update `SENSOR_MIN`, `SENSOR_MAX`, `DARK_LINE`, and `MIN_TOTAL_STRENGTH` in `include/config.h`.

## 2. Motor sanity check

With wheels lifted, verify positive commands move both wheels forward. Fix direction before tuning PID.

## 3. Tune proportional gain

Set `KI = 0` and `KD = 0`. Increase `KP` until the robot follows the line but begins to oscillate around it. Reduce slightly.

## 4. Add derivative damping

Increase `KD` gradually until oscillation and corner overshoot decrease. Excessive derivative gain makes the robot noisy and sensitive to sensor jitter.

## 5. Add integral only if required

Use a very small `KI` only if the robot shows a persistent one-sided bias that cannot be fixed mechanically. Integral is clamped to limit windup.

## 6. Increase speed

Raise `BASE_SPEED` in small steps and repeat the tuning process. Gains that work at low speed may not work at high speed.

## What to record

For credible project results, record track shape, battery voltage, motor/gearbox model, loop period, PID gains, lap time, failure count, and at least one telemetry trace. Do not claim benchmark performance until it has been measured on hardware.
