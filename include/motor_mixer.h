#pragma once

#include "embedded_compat.h"

struct MotorCommand {
  int left;
  int right;
  bool saturated;

  MotorCommand() : left(0), right(0), saturated(false) {}
  MotorCommand(int leftValue, int rightValue, bool saturatedValue)
      : left(leftValue), right(rightValue), saturated(saturatedValue) {}
};

inline MotorCommand mixDifferential(float base, float correction, int maxAbs) {
  float left = base + correction;
  float right = base - correction;
  const float peak = maxValue(absFloat(left), absFloat(right));
  bool saturated = false;
  if (peak > static_cast<float>(maxAbs) && peak > 0.0f) {
    const float scale = static_cast<float>(maxAbs) / peak;
    left *= scale;
    right *= scale;
    saturated = true;
  }
  return MotorCommand(roundToInt(left), roundToInt(right), saturated);
}
