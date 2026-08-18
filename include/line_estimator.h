#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

constexpr std::size_t LINE_SENSOR_COUNT = 5;

struct LineEstimatorConfig {
  std::array<int, LINE_SENSOR_COUNT> rawMin;
  std::array<int, LINE_SENSOR_COUNT> rawMax;
  std::array<int, LINE_SENSOR_COUNT> position;
  bool lineIsHigherRaw;
  int minTotalStrength;
  float minConfidence;
};

struct LineEstimate {
  float position = 0.0f;
  float confidence = 0.0f;
  int totalStrength = 0;
  bool visible = false;
  std::array<int, LINE_SENSOR_COUNT> normalized{};
};

class LineEstimator {
 public:
  explicit LineEstimator(LineEstimatorConfig cfg) : cfg_(cfg) {}

  LineEstimate estimate(const std::array<int, LINE_SENSOR_COUNT>& raw) const {
    LineEstimate result;
    long weighted = 0;
    int minStrength = 1000;
    int maxStrength = 0;

    for (std::size_t i = 0; i < LINE_SENSOR_COUNT; ++i) {
      const int strength = normalize(raw[i], cfg_.rawMin[i], cfg_.rawMax[i]);
      result.normalized[i] = strength;
      result.totalStrength += strength;
      weighted += static_cast<long>(strength) * cfg_.position[i];
      minStrength = std::min(minStrength, strength);
      maxStrength = std::max(maxStrength, strength);
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
    const int clipped = std::max(low, std::min(high, raw));
    int scaled = static_cast<int>((static_cast<long>(clipped - low) * 1000L) /
                                  (high - low));
    if (!cfg_.lineIsHigherRaw) scaled = 1000 - scaled;
    return std::max(0, std::min(1000, scaled));
  }

  static float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
  LineEstimatorConfig cfg_;
};
