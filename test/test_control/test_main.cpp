#include <unity.h>
#include "control.h"

void test_pwm_clamping() {
  TEST_ASSERT_EQUAL_INT(255, clampPwm(400));
  TEST_ASSERT_EQUAL_INT(-255, clampPwm(-400));
  TEST_ASSERT_EQUAL_INT(120, clampPwm(120));
}

void test_proportional_response() {
  PIDController pid({2.0f, 0.0f, 0.0f, 100.0f, 1.0f});
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 20.0f, pid.update(10.0f, 0.1f));
}

void test_integral_is_clamped() {
  PIDController pid({0.0f, 1.0f, 0.0f, 5.0f, 1.0f});
  for (int i = 0; i < 100; ++i) pid.update(10.0f, 1.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, pid.integral());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_pwm_clamping);
  RUN_TEST(test_proportional_response);
  RUN_TEST(test_integral_is_clamped);
  return UNITY_END();
}
