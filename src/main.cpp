#include <Arduino.h>

constexpr uint8_t SENSOR_PINS[5] = {A0, A1, A2, A3, A4};
constexpr int16_t SENSOR_POSITIONS[5] = {-2000, -1000, 0, 1000, 2000};
constexpr bool SENSOR_DARK_LINE = true;
constexpr int SENSOR_MIN = 100;
constexpr int SENSOR_MAX = 900;

constexpr uint8_t L_PWM = 5, L_IN1 = 7, L_IN2 = 8;
constexpr uint8_t R_PWM = 6, R_IN1 = 9, R_IN2 = 10;

float KP = 0.070f;
float KI = 0.00002f;
float KD = 0.42f;
int BASE_SPEED = 125;
constexpr int MAX_SPEED = 220;
constexpr uint16_t LOOP_MS = 8;

float integral = 0.0f;
float previousError = 0.0f;
float filteredDerivative = 0.0f;
int lastDirection = 1;
unsigned long previousTime = 0;

void driveMotor(uint8_t pwm, uint8_t in1, uint8_t in2, int speed) {
  speed = constrain(speed, -255, 255);
  digitalWrite(in1, speed >= 0 ? HIGH : LOW);
  digitalWrite(in2, speed >= 0 ? LOW : HIGH);
  analogWrite(pwm, abs(speed));
}

void setDrive(int left, int right) {
  driveMotor(L_PWM, L_IN1, L_IN2, left);
  driveMotor(R_PWM, R_IN1, R_IN2, right);
}

int normalizedStrength(int raw) {
  raw = constrain(raw, SENSOR_MIN, SENSOR_MAX);
  int scaled = map(raw, SENSOR_MIN, SENSOR_MAX, 0, 1000);
  return SENSOR_DARK_LINE ? scaled : 1000 - scaled;
}

bool readLine(float &error, int &confidence) {
  long weighted = 0;
  long total = 0;
  for (uint8_t i = 0; i < 5; ++i) {
    int strength = normalizedStrength(analogRead(SENSOR_PINS[i]));
    weighted += static_cast<long>(strength) * SENSOR_POSITIONS[i];
    total += strength;
  }
  confidence = total / 5;
  if (total < 350) return false;
  error = static_cast<float>(weighted) / static_cast<float>(total);
  if (error > 100) lastDirection = 1;
  if (error < -100) lastDirection = -1;
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(L_PWM, OUTPUT); pinMode(L_IN1, OUTPUT); pinMode(L_IN2, OUTPUT);
  pinMode(R_PWM, OUTPUT); pinMode(R_IN1, OUTPUT); pinMode(R_IN2, OUTPUT);
  previousTime = millis();
}

void loop() {
  unsigned long now = millis();
  if (now - previousTime < LOOP_MS) return;
  float dt = (now - previousTime) / 1000.0f;
  previousTime = now;

  float error = 0.0f;
  int confidence = 0;
  bool lineSeen = readLine(error, confidence);

  if (!lineSeen) {
    integral = 0.0f;
    setDrive(lastDirection > 0 ? 110 : -110, lastDirection > 0 ? -110 : 110);
    Serial.println("lost-line");
    return;
  }

  integral = constrain(integral + error * dt, -2500.0f, 2500.0f);
  float derivative = (error - previousError) / max(dt, 0.001f);
  filteredDerivative = 0.75f * filteredDerivative + 0.25f * derivative;
  previousError = error;

  float correction = KP * error + KI * integral + KD * filteredDerivative;
  int left = constrain(static_cast<int>(BASE_SPEED + correction), -MAX_SPEED, MAX_SPEED);
  int right = constrain(static_cast<int>(BASE_SPEED - correction), -MAX_SPEED, MAX_SPEED);
  setDrive(left, right);

  Serial.print("error="); Serial.print(error, 1);
  Serial.print(",confidence="); Serial.print(confidence);
  Serial.print(",left="); Serial.print(left);
  Serial.print(",right="); Serial.println(right);
}
