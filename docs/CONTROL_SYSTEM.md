# Control System

## Coordinates
Sensor positions are `[-2000,-1000,0,1000,2000]`. Positive position means the detected line is toward the right side under the default wiring convention.

## Normalization
Each sensor has independent `rawMin`/`rawMax`. Values map to `0..1000`. `LINE_IS_HIGHER_RAW` explicitly defines polarity instead of assuming all black/white reflectance boards have identical electrical behavior.

## Confidence
The estimator combines normalized signal energy with strongest-to-weakest channel contrast. `MIN_TOTAL_STRENGTH` and `MIN_CONFIDENCE` should be chosen from real sensor logs.

## PID
`PIDController` implements filtered derivative, integral limiting, output limiting and conditional-integration anti-windup. When output is saturated and error would drive farther into saturation, the integrator is frozen.

## Adaptive speed
Base speed falls as absolute line-position error rises and as confidence falls, providing additional steering authority without a separate curve classifier.

## Motor mixing
The requested pair starts as `left = base + correction`, `right = base - correction`. If either magnitude exceeds the configured limit, both are scaled by the same factor so steering ratio is preserved.

## Recovery
Recovery first rotates toward the last observed line direction, then alternates sweep direction. On reacquisition the PID is reset to avoid stale integral/derivative state.
