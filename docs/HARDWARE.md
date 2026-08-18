# Hardware Reference

Reference components: Arduino Uno/Nano-class controller, five analog reflectance channels, dual H-bridge, two geared brushed DC motors, and a motor-rated battery.

## Default pin map
| Signal | Pin |
|---|---:|
| sensors 0..4 | A0..A4 |
| left PWM | D5 |
| left IN1 / IN2 | D7 / D8 |
| right PWM | D6 |
| right IN1 / IN2 | D9 / D10 |

## Power
Motor current must not flow through the Arduino regulator. Use a separate motor-power path into the driver, common grounds where required, local decoupling near the driver, and short high-current loops.

## Sensor geometry
Keep the array rigid and parallel to the floor. Sensor height strongly changes contrast; record it with benchmark data. Physical left-to-right channel order must match `SENSOR_PINS` / `SENSOR_POSITION`.
