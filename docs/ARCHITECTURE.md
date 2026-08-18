# Architecture

## Design goals

The project separates hardware I/O from deterministic control logic so most behavior can be tested on a desktop without an Arduino attached.

## Runtime pipeline

1. `readSensors()` acquires five ADC channels.
2. `LineEstimator` normalizes each sensor independently and returns position, confidence and visibility.
3. If visible, `AdaptiveSpeedPlanner` selects base speed and `PIDController` calculates steering correction.
4. `mixDifferential()` scales the left/right pair together if either side exceeds the PWM limit.
5. If not visible, `RecoveryPlanner` overrides tracking with a staged search manoeuvre.
6. Motor commands are applied through the hardware-specific `drive()` layer.
7. Throttled telemetry publishes control state for offline analysis.

## Module boundary

| Module | Hardware independent | Native tested |
|---|---:|---:|
| PIDController | yes | yes |
| LineEstimator | yes | yes |
| AdaptiveSpeedPlanner | yes | yes |
| motor mixer | yes | yes |
| RecoveryPlanner | yes | yes |
| command parser | yes | yes |
| ADC/GPIO/PWM I/O | no | target build |

## Safety state

Firmware boots with `runEnabled = false`; motor PWM remains zero until `START`. `STOP` disables motion, clears PID/recovery state and commands both motors to zero. This software layer is not a substitute for a physical power disconnect.
