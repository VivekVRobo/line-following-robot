# Calibration

## 1. Verify motor direction
Lift the wheels. Keep firmware stopped. Verify positive motion and set `LEFT_MOTOR_REVERSED` / `RIGHT_MOTOR_REVERSED` rather than hiding polarity errors in controller signs.

## 2. Collect sensor samples
Record multiple readings over representative floor and line material at the intended sensor height. Use CSV columns `label,s0,s1,s2,s3,s4` and run:

```bash
python tools/calibrate_sensors.py calibration.csv
```

Paste the generated `SENSOR_MIN`, `SENSOR_MAX` and `LINE_IS_HIGHER_RAW` into `include/config.h`.

## 3. Validate visibility
Move the array across the line by hand while logging `confidence` and `total`. Choose thresholds from observations rather than guesses.

## 4. Tune control
1. Start at low speed.
2. Set `KI=0`.
3. Increase `KP` until tracking is responsive but begins to oscillate, then reduce slightly.
4. Increase `KD` to damp oscillation/overshoot.
5. Add only a small `KI` for persistent bias that cannot be fixed mechanically.
6. Increase straight speed gradually.
7. Check motor saturation ratio; chronic saturation means requested authority is unrealistic.

## 5. Recalibrate after changes
Repeat after changing sensor height, track material, supply voltage, motors, wheels, driver, or sensor board.
