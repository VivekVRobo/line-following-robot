# Troubleshooting

## Robot spins after START
Check motor reversal flags and verify a centered line produces near-zero position.

## Position sign is backwards
Confirm sensor ordering matches `SENSOR_POSITION`; fix ordering rather than compensating with arbitrary PID signs.

## Line always lost
Run calibration, inspect `total` / `confidence`, and verify `LINE_IS_HIGHER_RAW`.

## Rapid oscillation
Reduce speed or `KP`; add `KD` gradually. Confirm the loop remains near 10 ms.

## Slow on straights
Inspect confidence. The speed planner deliberately slows weak detections; fix sensing before raising speed.

## Frequent max PWM
Use the telemetry analyzer. Chronic saturation means gains/speed exceed available motor authority.

## Recovery searches wrong direction first
Verify the line-position sign convention and physical left/right channel order.
