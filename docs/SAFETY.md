# Safety

- Firmware boots with motor motion disabled; send `START` only in a safe test area.
- Keep a physical battery disconnect or power switch accessible.
- Never power DC motors from the microcontroller 5 V rail.
- Use a motor supply and driver rated above measured stall current.
- Test motor polarity with wheels lifted.
- Keep fingers, wires, hair and clothing away from wheels/gears.
- Stop if the driver, wiring, battery, connector or motor becomes unusually hot.
- The software `STOP` command is useful but is not a certified emergency-stop system.
