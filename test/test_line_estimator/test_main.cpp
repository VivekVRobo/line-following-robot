#include <unity.h>
#include "line_estimator.h"

LineEstimator makeEstimator(bool higher=true){ return LineEstimator({{100,100,100,100,100},{900,900,900,900,900},{-2000,-1000,0,1000,2000},higher,500,0.10f}); }
void test_centered_line_estimates_center(){ auto e=makeEstimator(); auto r=e.estimate({100,150,900,150,100}); TEST_ASSERT_TRUE(r.visible); TEST_ASSERT_FLOAT_WITHIN(80.0f,0.0f,r.position); }
void test_right_line_has_positive_position(){ auto e=makeEstimator(); auto r=e.estimate({100,100,200,700,900}); TEST_ASSERT_TRUE(r.position>700.0f); }
void test_low_energy_is_not_visible(){ auto e=makeEstimator(); auto r=e.estimate({100,100,100,100,100}); TEST_ASSERT_FALSE(r.visible); }
void test_low_raw_polarity_can_be_selected(){ auto e=makeEstimator(false); auto r=e.estimate({900,850,100,850,900}); TEST_ASSERT_TRUE(r.visible); TEST_ASSERT_FLOAT_WITHIN(100.0f,0.0f,r.position); }
int main(int,char**){ UNITY_BEGIN(); RUN_TEST(test_centered_line_estimates_center); RUN_TEST(test_right_line_has_positive_position); RUN_TEST(test_low_energy_is_not_visible); RUN_TEST(test_low_raw_polarity_can_be_selected); return UNITY_END(); }
