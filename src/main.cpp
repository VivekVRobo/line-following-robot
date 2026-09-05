#include <Arduino.h>

#include "command_parser.h"
#include "config.h"
#include "control.h"
#include "line_estimator.h"
#include "motor_mixer.h"
#include "recovery.h"
#include "speed_profile.h"

namespace {
PIDController pid({cfg::KP, cfg::KI, cfg::KD, cfg::INTEGRAL_LIMIT, cfg::DERIVATIVE_ALPHA, cfg::PID_OUTPUT_LIMIT});
LineEstimator estimator({cfg::SENSOR_MIN, cfg::SENSOR_MAX, cfg::SENSOR_POSITION, cfg::LINE_IS_HIGHER_RAW, cfg::MIN_TOTAL_STRENGTH, cfg::MIN_CONFIDENCE});
AdaptiveSpeedPlanner speedPlanner({static_cast<float>(cfg::STRAIGHT_SPEED), static_cast<float>(cfg::MIN_TRACKING_SPEED), 2000.0f, cfg::CURVATURE_SLOWDOWN, cfg::LOW_CONFIDENCE_SLOWDOWN});
RecoveryPlanner recovery({cfg::RECOVERY_SPIN_SPEED, cfg::RECOVERY_SWEEP_SPEED, cfg::RECOVERY_INITIAL_SPIN_MS, cfg::RECOVERY_SWEEP_PERIOD_MS});

bool runEnabled = false;
bool telemetryEnabled = true;
int lastDirection = 1;
uint32_t lastLoopMs = 0;
uint32_t lastTelemetryMs = 0;
char commandBuffer[32] = {};
uint8_t commandLength = 0;

void driveOne(uint8_t pwmPin, uint8_t in1, uint8_t in2, int command, bool reversed) {
  if (reversed) command = -command;
  command = constrain(command, -cfg::MAX_SPEED, cfg::MAX_SPEED);
  if (command == 0) {
    digitalWrite(in1, LOW); digitalWrite(in2, LOW); analogWrite(pwmPin, 0); return;
  }
  const bool forward = command > 0;
  digitalWrite(in1, forward ? HIGH : LOW);
  digitalWrite(in2, forward ? LOW : HIGH);
  analogWrite(pwmPin, abs(command));
}

void drive(int left, int right) {
  driveOne(cfg::LEFT_PWM, cfg::LEFT_IN1, cfg::LEFT_IN2, left, cfg::LEFT_MOTOR_REVERSED);
  driveOne(cfg::RIGHT_PWM, cfg::RIGHT_IN1, cfg::RIGHT_IN2, right, cfg::RIGHT_MOTOR_REVERSED);
}

FixedArray<int, LINE_SENSOR_COUNT> readSensors() {
  FixedArray<int, LINE_SENSOR_COUNT> raw{};
  for (uint8_t i = 0; i < cfg::SENSOR_COUNT; ++i) raw[i] = analogRead(cfg::SENSOR_PINS[i]);
  return raw;
}

void printHelp() { Serial.println(F("commands: START | STOP | STATUS | TELEM ON | TELEM OFF | HELP")); }
void printStatus() {
  Serial.print(F("status,run=")); Serial.print(runEnabled ? 1 : 0);
  Serial.print(F(",telemetry=")); Serial.print(telemetryEnabled ? 1 : 0);
  Serial.print(F(",recovery=")); Serial.println(recovery.active() ? 1 : 0);
}

void applyCommand(CommandType command) {
  switch (command) {
    case CommandType::Start: pid.reset(); recovery.reset(); runEnabled = true; Serial.println(F("ack,START")); break;
    case CommandType::Stop: runEnabled = false; pid.reset(); recovery.reset(); drive(0, 0); Serial.println(F("ack,STOP")); break;
    case CommandType::TelemetryOn: telemetryEnabled = true; Serial.println(F("ack,TELEM ON")); break;
    case CommandType::TelemetryOff: telemetryEnabled = false; Serial.println(F("ack,TELEM OFF")); break;
    case CommandType::Status: printStatus(); break;
    case CommandType::Help: printHelp(); break;
    case CommandType::Unknown: Serial.println(F("err,unknown-command")); break;
    case CommandType::None: break;
  }
}

void serviceSerial() {
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r') continue;
    if (c == '\n') {
      commandBuffer[commandLength] = '\0';
      applyCommand(parseCommand(commandBuffer));
      commandLength = 0; commandBuffer[0] = '\0'; continue;
    }
    if (commandLength < sizeof(commandBuffer) - 1) commandBuffer[commandLength++] = c;
    else { commandLength = 0; Serial.println(F("err,command-too-long")); }
  }
}

void emitTelemetry(uint32_t now, const char* mode, const LineEstimate& line, float error,
                   float correction, int base, const MotorCommand& motors, uint8_t recoveryPhase = 0) {
  if (!telemetryEnabled || now - lastTelemetryMs < cfg::TELEMETRY_PERIOD_MS) return;
  lastTelemetryMs = now;
  Serial.print(F("T,")); Serial.print(now); Serial.print(','); Serial.print(mode);
  Serial.print(','); Serial.print(line.position, 1); Serial.print(','); Serial.print(line.confidence, 3);
  Serial.print(','); Serial.print(line.totalStrength); Serial.print(','); Serial.print(error, 1);
  Serial.print(','); Serial.print(correction, 2); Serial.print(','); Serial.print(base);
  Serial.print(','); Serial.print(motors.left); Serial.print(','); Serial.print(motors.right);
  Serial.print(','); Serial.println(recoveryPhase);
}
}

void setup() {
  Serial.begin(cfg::SERIAL_BAUD);
  const uint8_t motorPins[] = {
      cfg::LEFT_PWM, cfg::LEFT_IN1, cfg::LEFT_IN2,
      cfg::RIGHT_PWM, cfg::RIGHT_IN1, cfg::RIGHT_IN2};
  for (size_t i = 0; i < sizeof(motorPins) / sizeof(motorPins[0]); ++i) {
    pinMode(motorPins[i], OUTPUT);
  }
  drive(0, 0);
  lastLoopMs = millis();
  Serial.println(F("line-follower,v2,SAFE-STOP"));
  Serial.println(F("# T,ms,mode,position,confidence,total,error,correction,base,left,right,recovery_phase"));
  printHelp();
}

void loop() {
  serviceSerial();
  const uint32_t now = millis();
  if (now - lastLoopMs < cfg::LOOP_PERIOD_MS) return;
  const float dt = static_cast<float>(now - lastLoopMs) / 1000.0f;
  lastLoopMs = now;
  if (!runEnabled) { drive(0, 0); return; }

  const LineEstimate line = estimator.estimate(readSensors());
  if (!line.visible) {
    if (!recovery.active()) recovery.begin(lastDirection, now);
    pid.reset();
    const RecoveryCommand search = recovery.command(now);
    drive(search.left, search.right);
    const MotorCommand recoveryMotors(search.left, search.right, false);
    emitTelemetry(now, "RECOVER", line, 0.0f, 0.0f, 0, recoveryMotors, search.phase);
    return;
  }

  if (recovery.active()) { recovery.reset(); pid.reset(); }
  const float error = line.position;
  if (error > 70.0f) lastDirection = 1; else if (error < -70.0f) lastDirection = -1;
  const int base = speedPlanner.command(error, line.confidence);
  const float correction = pid.update(error, dt);
  const MotorCommand motors = mixDifferential(static_cast<float>(base), correction, cfg::MAX_SPEED);
  drive(motors.left, motors.right);
  emitTelemetry(now, "TRACK", line, error, correction, base, motors);
}
