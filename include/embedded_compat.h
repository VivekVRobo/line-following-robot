#pragma once

#include <stddef.h>
#include <stdint.h>

template <typename T, size_t N>
struct FixedArray {
  T values[N];

  T& operator[](size_t index) { return values[index]; }
  const T& operator[](size_t index) const { return values[index]; }
};

template <typename T>
constexpr T minValue(T a, T b) {
  return a < b ? a : b;
}

template <typename T>
constexpr T maxValue(T a, T b) {
  return a > b ? a : b;
}

template <typename T>
constexpr T clampValue(T value, T low, T high) {
  return maxValue(low, minValue(high, value));
}

constexpr float absFloat(float value) {
  return value < 0.0f ? -value : value;
}

constexpr int roundToInt(float value) {
  return static_cast<int>(value >= 0.0f ? value + 0.5f : value - 0.5f);
}
