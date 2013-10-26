/*
 * batch_test.cpp
 *
 *  Created on: Oct 25, 2013
 *      Author: dhardman
 */

#include "common/batch.h"
#include "gtest/gtest.h"

using namespace mtm;

TEST(BatchTest, nonexistent_file) {
	Batch b("doesn't@exist");
	ASSERT_EQ(NULL, b.next_line());
	ASSERT_DOUBLE_EQ(1.0, b.ratio_complete());
}
