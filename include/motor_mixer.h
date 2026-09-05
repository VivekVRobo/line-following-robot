#pragma once

#include "embedded_compat.h"

struct MotorCommand {
  int left = 0;
  int right = 0;
  bool saturated = false;
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
  return {roundToInt(left), roundToInt(right), saturated};
}
