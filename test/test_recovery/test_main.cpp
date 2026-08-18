#include <unity.h>
#include "recovery.h"
void test_recovery_prefers_last_direction_first(){ RecoveryPlanner p({100,120,300,500}); p.begin(1,1000); auto c=p.command(1100); TEST_ASSERT_EQUAL_INT(100,c.left); TEST_ASSERT_EQUAL_INT(-100,c.right); TEST_ASSERT_EQUAL_UINT8(1,c.phase); }
void test_recovery_sweeps_after_initial_spin(){ RecoveryPlanner p({100,120,300,500}); p.begin(1,1000); auto a=p.command(1400); auto b=p.command(1900); TEST_ASSERT_EQUAL_INT(120,a.left); TEST_ASSERT_EQUAL_INT(-120,a.right); TEST_ASSERT_EQUAL_INT(-120,b.left); TEST_ASSERT_EQUAL_INT(120,b.right); }
int main(int,char**){ UNITY_BEGIN(); RUN_TEST(test_recovery_prefers_last_direction_first); RUN_TEST(test_recovery_sweeps_after_initial_spin); return UNITY_END(); }
