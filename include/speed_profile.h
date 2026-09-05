#pragma once

#include "embedded_compat.h"

struct SpeedProfileConfig {
  float straightSpeed;
  float minimumTrackingSpeed;
  float maxPosition;
  float curvatureSlowdown;
  float lowConfidenceSlowdown;
};

class AdaptiveSpeedPlanner {
 public:
  explicit AdaptiveSpeedPlanner(SpeedProfileConfig cfg) : cfg_(cfg) {}

  int command(float positionError, float confidence) const {
    const float normalizedError = clamp01(absFloat(positionError) / cfg_.maxPosition);
    const float confidencePenalty = 1.0f - clamp01(confidence);
    const float slowdown = cfg_.curvatureSlowdown * normalizedError +
                           cfg_.lowConfidenceSlowdown * confidencePenalty;
    const float speed = maxValue(cfg_.minimumTrackingSpeed,
                                 cfg_.straightSpeed * (1.0f - minValue(0.85f, slowdown)));
    return roundToInt(speed);
  }

 private:
  static float clamp01(float v) { return clampValue(v, 0.0f, 1.0f); }
  SpeedProfileConfig cfg_;
};
