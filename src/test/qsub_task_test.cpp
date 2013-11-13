#define _PROPERLY_INCLUDED
#include "domain/qsub_task.h"

#include "gtest/gtest.h"

using namespace nitro;

TEST(qsub_task_test, walltime_to_seconds) {
	EXPECT_EQ(0.0, walltime_to_seconds("0"));
	EXPECT_EQ(100.0, walltime_to_seconds("100"));
	EXPECT_EQ(-5.0, walltime_to_seconds("-5"));
	EXPECT_EQ(25.0, walltime_to_seconds("0:25"));
	EXPECT_EQ(5103.0, walltime_to_seconds("1:25:3"));
	EXPECT_EQ(93784.0, walltime_to_seconds("1:2:3:4"));
}
