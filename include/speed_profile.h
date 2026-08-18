#pragma once

#include <algorithm>
#include <cmath>

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
    const float normalizedError = clamp01(std::fabs(positionError) / cfg_.maxPosition);
    const float confidencePenalty = 1.0f - clamp01(confidence);
    const float slowdown = cfg_.curvatureSlowdown * normalizedError +
                           cfg_.lowConfidenceSlowdown * confidencePenalty;
    const float speed = std::max(cfg_.minimumTrackingSpeed,
                                 cfg_.straightSpeed * (1.0f - std::min(0.85f, slowdown)));
    return static_cast<int>(std::lround(speed));
  }

 private:
  static float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
  SpeedProfileConfig cfg_;
};
