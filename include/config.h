#pragma once

#include <Arduino.h>

namespace cfg {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t LOOP_PERIOD_MS = 10;

constexpr uint8_t SENSOR_COUNT = 5;
constexpr uint8_t SENSOR_PINS[SENSOR_COUNT] = {A0, A1, A2, A3, A4};
constexpr int16_t SENSOR_POSITION[SENSOR_COUNT] = {-2000, -1000, 0, 1000, 2000};

// Replace these with measurements from the actual sensor array.
constexpr int SENSOR_MIN = 120;
constexpr int SENSOR_MAX = 900;
constexpr bool DARK_LINE = true;
constexpr int MIN_TOTAL_STRENGTH = 120;

constexpr uint8_t LEFT_PWM = 5;
constexpr uint8_t LEFT_IN1 = 7;
constexpr uint8_t LEFT_IN2 = 8;
constexpr uint8_t RIGHT_PWM = 6;
constexpr uint8_t RIGHT_IN1 = 9;
constexpr uint8_t RIGHT_IN2 = 10;

constexpr int BASE_SPEED = 125;
constexpr int MAX_SPEED = 220;
constexpr int SEARCH_SPEED = 105;

constexpr float KP = 0.085f;
constexpr float KI = 0.0008f;
constexpr float KD = 0.30f;
constexpr float INTEGRAL_LIMIT = 4500.0f;
constexpr float DERIVATIVE_ALPHA = 0.35f;
}
