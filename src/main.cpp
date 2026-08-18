#include <Arduino.h>
#include "config.h"
#include "control.h"

PIDController pid({cfg::KP, cfg::KI, cfg::KD, cfg::INTEGRAL_LIMIT, cfg::DERIVATIVE_ALPHA});
int lastDirection = 1;
uint32_t lastLoopMs = 0;

struct LineSample {
  int position;
  int totalStrength;
  bool visible;
};

int normalizeSensor(int raw) {
  const int constrainedRaw = constrain(raw, cfg::SENSOR_MIN, cfg::SENSOR_MAX);
  long normalized = map(constrainedRaw, cfg::SENSOR_MIN, cfg::SENSOR_MAX, 0, 1000);
  if (!cfg::DARK_LINE) normalized = 1000 - normalized;
  return constrain(static_cast<int>(normalized), 0, 1000);
}

LineSample readLine() {
  long weighted = 0;
  long total = 0;

  for (uint8_t i = 0; i < cfg::SENSOR_COUNT; ++i) {
    const int strength = normalizeSensor(analogRead(cfg::SENSOR_PINS[i]));
    weighted += static_cast<long>(strength) * cfg::SENSOR_POSITION[i];
    total += strength;
  }

  if (total < cfg::MIN_TOTAL_STRENGTH) return {0, static_cast<int>(total), false};
  return {static_cast<int>(weighted / total), static_cast<int>(total), true};
}

void driveOne(uint8_t pwmPin, uint8_t in1, uint8_t in2, int command) {
  command = clampPwm(command, cfg::MAX_SPEED);
  const bool forward = command >= 0;
  digitalWrite(in1, forward ? HIGH : LOW);
  digitalWrite(in2, forward ? LOW : HIGH);
  analogWrite(pwmPin, abs(command));
}

void drive(int left, int right) {
  driveOne(cfg::LEFT_PWM, cfg::LEFT_IN1, cfg::LEFT_IN2, left);
  driveOne(cfg::RIGHT_PWM, cfg::RIGHT_IN1, cfg::RIGHT_IN2, right);
}

void searchForLine() {
  pid.reset();
  if (lastDirection >= 0) drive(cfg::SEARCH_SPEED, -cfg::SEARCH_SPEED);
  else drive(-cfg::SEARCH_SPEED, cfg::SEARCH_SPEED);
}

void setup() {
  Serial.begin(cfg::SERIAL_BAUD);
  for (uint8_t pin : {cfg::LEFT_PWM, cfg::LEFT_IN1, cfg::LEFT_IN2,
                      cfg::RIGHT_PWM, cfg::RIGHT_IN1, cfg::RIGHT_IN2}) {
    pinMode(pin, OUTPUT);
  }
  lastLoopMs = millis();
  Serial.println(F("line-follower ready"));
}

void loop() {
  const uint32_t now = millis();
  if (now - lastLoopMs < cfg::LOOP_PERIOD_MS) return;
  const float dt = (now - lastLoopMs) / 1000.0f;
  lastLoopMs = now;

  const LineSample line = readLine();
  if (!line.visible) {
    searchForLine();
    Serial.println(F("lost-line"));
    return;
  }

  const float error = static_cast<float>(line.position);
  if (error > 80) lastDirection = 1;
  else if (error < -80) lastDirection = -1;

  const int correction = static_cast<int>(pid.update(error, dt));
  const int left = clampPwm(cfg::BASE_SPEED + correction, cfg::MAX_SPEED);
  const int right = clampPwm(cfg::BASE_SPEED - correction, cfg::MAX_SPEED);
  drive(left, right);

  Serial.print(F("pos=")); Serial.print(line.position);
  Serial.print(F(",strength=")); Serial.print(line.totalStrength);
  Serial.print(F(",corr=")); Serial.print(correction);
  Serial.print(F(",left=")); Serial.print(left);
  Serial.print(F(",right=")); Serial.println(right);
}
