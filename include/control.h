#pragma once

#include "embedded_compat.h"

struct PIDConfig {
  float kp;
  float ki;
  float kd;
  float integralLimit;
  float derivativeAlpha;
  float outputLimit;
};

struct PIDTerms {
  float p;
  float i;
  float d;
  float output;

  PIDTerms() : p(0.0f), i(0.0f), d(0.0f), output(0.0f) {}
  PIDTerms(float pValue, float iValue, float dValue, float outputValue)
      : p(pValue), i(iValue), d(dValue), output(outputValue) {}
};

class PIDController {
 public:
  explicit PIDController(PIDConfig cfg) : cfg_(cfg) {}

  float update(float error, float dtSeconds) {
    if (dtSeconds <= 0.0f) return terms_.output;

    const float rawDerivative = initialized_ ? (error - previousError_) / dtSeconds : 0.0f;
    filteredDerivative_ = cfg_.derivativeAlpha * rawDerivative +
                          (1.0f - cfg_.derivativeAlpha) * filteredDerivative_;

    float candidateIntegral = integral_ + error * dtSeconds;
    candidateIntegral = clamp(candidateIntegral, -cfg_.integralLimit, cfg_.integralLimit);

    const float p = cfg_.kp * error;
    const float d = cfg_.kd * filteredDerivative_;
    const float candidate = p + cfg_.ki * candidateIntegral + d;
    const float limitedCandidate = clamp(candidate, -cfg_.outputLimit, cfg_.outputLimit);

    const bool saturated = absFloat(candidate - limitedCandidate) > 1e-6f;
    const bool drivesFurtherIntoSaturation = (candidate * error) > 0.0f;
    if (!(saturated && drivesFurtherIntoSaturation)) integral_ = candidateIntegral;

    const float output = clamp(p + cfg_.ki * integral_ + d,
                               -cfg_.outputLimit, cfg_.outputLimit);
    previousError_ = error;
    initialized_ = true;
    terms_ = PIDTerms(p, cfg_.ki * integral_, d, output);
    return output;
  }

  void reset() {
    integral_ = 0.0f;
    previousError_ = 0.0f;
    filteredDerivative_ = 0.0f;
    initialized_ = false;
    terms_ = PIDTerms();
  }

  void setTunings(float kp, float ki, float kd) {
    cfg_.kp = kp;
    cfg_.ki = ki;
    cfg_.kd = kd;
  }

  PIDTerms terms() const { return terms_; }
  float integral() const { return integral_; }

 private:
  static float clamp(float value, float low, float high) {
    return clampValue(value, low, high);
  }

  PIDConfig cfg_;
  PIDTerms terms_;
  float integral_ = 0.0f;
  float previousError_ = 0.0f;
  float filteredDerivative_ = 0.0f;
  bool initialized_ = false;
};
