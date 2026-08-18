# Hardware Integration

## Reference parts

- Arduino Uno/Nano compatible microcontroller
- 5-channel analog IR reflectance array
- Dual H-bridge motor driver such as TB6612FNG
- Two geared DC motors
- Battery sized for motor voltage/current
- Optional switch/fuse appropriate to the battery

## Power rules

1. Do not run the motors from the Arduino regulator.
2. Connect motor supply directly to the motor driver's VM input.
3. Power logic according to the driver's datasheet.
4. Use a shared ground between MCU and motor driver.
5. Add local decoupling close to the driver and controller.

## First power-up

- Disconnect/raise wheels.
- Confirm no short between supply and ground.
- Power logic before commanding motion.
- Test each motor at low PWM.
- If one motor runs backward, correct wiring or direction mapping before track testing.

## Sensor placement

Keep the array close enough to the surface for repeatable contrast, mount it perpendicular to the travel direction, and ensure the center sensor aligns with the robot centerline. Record raw readings over both the track background and line before changing thresholds.
