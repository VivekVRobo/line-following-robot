#pragma once

#include <Arduino.h>
#include <array>

namespace cfg {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t LOOP_PERIOD_MS = 10;
constexpr uint16_t TELEMETRY_PERIOD_MS = 50;

constexpr uint8_t SENSOR_COUNT = 5;
constexpr uint8_t SENSOR_PINS[SENSOR_COUNT] = {A0, A1, A2, A3, A4};
constexpr std::array<int, SENSOR_COUNT> SENSOR_POSITION = {-2000, -1000, 0, 1000, 2000};

// Replace these values with output from tools/calibrate_sensors.py.
constexpr std::array<int, SENSOR_COUNT> SENSOR_MIN = {120, 120, 120, 120, 120};
constexpr std::array<int, SENSOR_COUNT> SENSOR_MAX = {900, 900, 900, 900, 900};
constexpr bool LINE_IS_HIGHER_RAW = true;
constexpr int MIN_TOTAL_STRENGTH = 650;
constexpr float MIN_CONFIDENCE = 0.16f;

constexpr uint8_t LEFT_PWM = 5;
constexpr uint8_t LEFT_IN1 = 7;
constexpr uint8_t LEFT_IN2 = 8;
constexpr uint8_t RIGHT_PWM = 6;
constexpr uint8_t RIGHT_IN1 = 9;
constexpr uint8_t RIGHT_IN2 = 10;
constexpr bool LEFT_MOTOR_REVERSED = false;
constexpr bool RIGHT_MOTOR_REVERSED = false;

constexpr int MAX_SPEED = 220;
constexpr int STRAIGHT_SPEED = 145;
constexpr int MIN_TRACKING_SPEED = 90;
constexpr float CURVATURE_SLOWDOWN = 0.48f;
constexpr float LOW_CONFIDENCE_SLOWDOWN = 0.30f;

constexpr float KP = 0.085f;
constexpr float KI = 0.0008f;
constexpr float KD = 0.30f;
constexpr float INTEGRAL_LIMIT = 4500.0f;
constexpr float DERIVATIVE_ALPHA = 0.35f;
constexpr float PID_OUTPUT_LIMIT = 190.0f;

constexpr int RECOVERY_SPIN_SPEED = 105;
constexpr int RECOVERY_SWEEP_SPEED = 125;
constexpr uint32_t RECOVERY_INITIAL_SPIN_MS = 350;
constexpr uint32_t RECOVERY_SWEEP_PERIOD_MS = 600;
}
