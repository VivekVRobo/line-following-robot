#pragma once

#include <algorithm>
#include <cmath>

struct MotorCommand {
  int left = 0;
  int right = 0;
  bool saturated = false;
};

inline MotorCommand mixDifferential(float base, float correction, int maxAbs) {
  float left = base + correction;
  float right = base - correction;
  const float peak = std::max(std::fabs(left), std::fabs(right));
  bool saturated = false;
  if (peak > static_cast<float>(maxAbs) && peak > 0.0f) {
    const float scale = static_cast<float>(maxAbs) / peak;
    left *= scale;
    right *= scale;
    saturated = true;
  }
  return {static_cast<int>(std::lround(left)), static_cast<int>(std::lround(right)), saturated};
}
