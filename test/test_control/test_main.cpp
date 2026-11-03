#include <math.h>
#include <unity.h>

#include "control.h"
#include "motor_mixer.h"
#include "speed_profile.h"
#include "command_parser.h"

void test_pid_zero_error_zero_output() { PIDController pid({1.0f,0.1f,0.2f,100.0f,0.5f,50.0f}); TEST_ASSERT_FLOAT_WITHIN(0.001f,0.0f,pid.update(0.0f,0.01f)); }
void test_pid_output_is_limited() { PIDController pid({10.0f,5.0f,0.0f,1000.0f,0.5f,25.0f}); TEST_ASSERT_FLOAT_WITHIN(0.001f,25.0f,pid.update(100.0f,0.1f)); }
void test_pid_anti_windup_freezes_integral_when_saturated() { PIDController pid({10.0f,10.0f,0.0f,1000.0f,0.5f,20.0f}); for(int i=0;i<20;++i) pid.update(100.0f,0.1f); TEST_ASSERT_FLOAT_WITHIN(0.001f,0.0f,pid.integral()); }

void test_pid_rejects_non_finite_samples_without_poisoning_state() {
  PIDController pid({1.0f,0.1f,0.2f,100.0f,0.5f,50.0f});
  const float previousOutput = pid.update(2.0f, 0.1f);
  const float previousIntegral = pid.integral();
  const PIDTerms previousTerms = pid.terms();

  TEST_ASSERT_TRUE(isfinite(previousOutput));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, previousOutput, pid.update(NAN, 0.1f));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, previousOutput, pid.update(INFINITY, 0.1f));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, previousOutput, pid.update(3.0f, NAN));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, previousOutput, pid.update(3.0f, INFINITY));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, previousIntegral, pid.integral());

  const PIDTerms currentTerms = pid.terms();
  TEST_ASSERT_FLOAT_WITHIN(0.001f, previousTerms.p, currentTerms.p);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, previousTerms.i, currentTerms.i);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, previousTerms.d, currentTerms.d);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, previousTerms.output, currentTerms.output);
  TEST_ASSERT_TRUE(isfinite(pid.update(1.0f, 0.1f)));
}

void test_pid_ignores_non_finite_tuning_updates() {
  PIDController pid({1.0f,0.5f,0.2f,100.0f,0.5f,50.0f});
  pid.setTunings(NAN, 0.0f, 0.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.1f, pid.update(2.0f, 0.1f));

  pid.setTunings(2.0f, 0.0f, 0.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 4.0f, pid.update(2.0f, 0.1f));
}

void test_motor_mixer_preserves_ratio_under_saturation() { const MotorCommand cmd=mixDifferential(180.0f,100.0f,220); TEST_ASSERT_TRUE(cmd.saturated); TEST_ASSERT_EQUAL_INT(220,cmd.left); TEST_ASSERT_TRUE(cmd.right>0); TEST_ASSERT_TRUE(cmd.right<cmd.left); }
void test_adaptive_speed_slows_on_large_error() { AdaptiveSpeedPlanner planner({150.0f,80.0f,2000.0f,0.5f,0.3f}); const int straight=planner.command(0.0f,1.0f); const int curve=planner.command(1800.0f,1.0f); TEST_ASSERT_TRUE(curve<straight); TEST_ASSERT_TRUE(curve>=80); }
void test_command_parser() { TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Start),static_cast<int>(parseCommand("start"))); TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::TelemetryOff),static_cast<int>(parseCommand("TELEM OFF"))); TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Unknown),static_cast<int>(parseCommand("GO FAST"))); }

int main(int,char**){
  UNITY_BEGIN();
  RUN_TEST(test_pid_zero_error_zero_output);
  RUN_TEST(test_pid_output_is_limited);
  RUN_TEST(test_pid_anti_windup_freezes_integral_when_saturated);
  RUN_TEST(test_pid_rejects_non_finite_samples_without_poisoning_state);
  RUN_TEST(test_pid_ignores_non_finite_tuning_updates);
  RUN_TEST(test_motor_mixer_preserves_ratio_under_saturation);
  RUN_TEST(test_adaptive_speed_slows_on_large_error);
  RUN_TEST(test_command_parser);
  return UNITY_END();
}
