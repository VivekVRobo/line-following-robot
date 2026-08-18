#pragma once

#include <algorithm>

struct PIDConfig {
  float kp;
  float ki;
  float kd;
  float integralLimit;
  float derivativeAlpha;
};

class PIDController {
 public:
  explicit PIDController(PIDConfig cfg) : cfg_(cfg) {}

  float update(float error, float dtSeconds) {
    if (dtSeconds <= 0.0f) return 0.0f;

    integral_ += error * dtSeconds;
    integral_ = std::max(-cfg_.integralLimit, std::min(cfg_.integralLimit, integral_));

    const float rawDerivative = initialized_ ? (error - previousError_) / dtSeconds : 0.0f;
    filteredDerivative_ = cfg_.derivativeAlpha * rawDerivative +
                          (1.0f - cfg_.derivativeAlpha) * filteredDerivative_;

    previousError_ = error;
    initialized_ = true;
    return cfg_.kp * error + cfg_.ki * integral_ + cfg_.kd * filteredDerivative_;
  }

  void reset() {
    integral_ = 0.0f;
    previousError_ = 0.0f;
    filteredDerivative_ = 0.0f;
    initialized_ = false;
  }

  float integral() const { return integral_; }

 private:
  PIDConfig cfg_;
  float integral_ = 0.0f;
  float previousError_ = 0.0f;
  float filteredDerivative_ = 0.0f;
  bool initialized_ = false;
};

inline int clampPwm(int value, int maxAbs = 255) {
  return std::max(-maxAbs, std::min(maxAbs, value));
}
