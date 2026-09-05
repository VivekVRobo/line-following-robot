#pragma once

#include "embedded_compat.h"

constexpr size_t LINE_SENSOR_COUNT = 5;

struct LineEstimatorConfig {
  FixedArray<int, LINE_SENSOR_COUNT> rawMin;
  FixedArray<int, LINE_SENSOR_COUNT> rawMax;
  FixedArray<int, LINE_SENSOR_COUNT> position;
  bool lineIsHigherRaw;
  int minTotalStrength;
  float minConfidence;
};

struct LineEstimate {
  float position = 0.0f;
  float confidence = 0.0f;
  int totalStrength = 0;
  bool visible = false;
  FixedArray<int, LINE_SENSOR_COUNT> normalized{};
};

class LineEstimator {
 public:
  explicit LineEstimator(LineEstimatorConfig cfg) : cfg_(cfg) {}

  LineEstimate estimate(const FixedArray<int, LINE_SENSOR_COUNT>& raw) const {
    LineEstimate result;
    long weighted = 0;
    int minStrength = 1000;
    int maxStrength = 0;

    for (size_t i = 0; i < LINE_SENSOR_COUNT; ++i) {
      const int strength = normalize(raw[i], cfg_.rawMin[i], cfg_.rawMax[i]);
      result.normalized[i] = strength;
      result.totalStrength += strength;
      weighted += static_cast<long>(strength) * cfg_.position[i];
      minStrength = minValue(minStrength, strength);
      maxStrength = maxValue(maxStrength, strength);
    }

    if (result.totalStrength > 0) {
      result.position = static_cast<float>(weighted) / static_cast<float>(result.totalStrength);
    }

    const float energy = static_cast<float>(result.totalStrength) /
                         static_cast<float>(LINE_SENSOR_COUNT * 1000);
    const float contrast = static_cast<float>(maxStrength - minStrength) / 1000.0f;
    result.confidence = clamp01(0.65f * energy + 0.35f * contrast);
    result.visible = result.totalStrength >= cfg_.minTotalStrength &&
                     result.confidence >= cfg_.minConfidence;
    return result;
  }

 private:
  int normalize(int raw, int low, int high) const {
    if (high <= low) return 0;
    const int clipped = clampValue(raw, low, high);
    int scaled = static_cast<int>((static_cast<long>(clipped - low) * 1000L) /
                                  (high - low));
    if (!cfg_.lineIsHigherRaw) scaled = 1000 - scaled;
    return clampValue(scaled, 0, 1000);
  }

  static float clamp01(float v) { return clampValue(v, 0.0f, 1.0f); }
  LineEstimatorConfig cfg_;
};
