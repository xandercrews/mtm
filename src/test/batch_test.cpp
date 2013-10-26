/*
 * batch_test.cpp
 *
 *  Created on: Oct 25, 2013
 *      Author: dhardman
 */

#include "gtest/gtest.h"
#include "common/batch.h"
#include "common/error.h"

using namespace mtm;
using namespace mtm::event;

TEST(BatchTest, nonexistent_file) {
	Batch b("doesn't@exist");
	ASSERT_EQ(NULL, b.next_line());
	ASSERT_DOUBLE_EQ(1.0, b.ratio_complete());
}

TEST(BatchTest, binary_file) {
	try {
		Batch b("proc/self/exe");
	} catch (Error const & e) {
		EXPECT_EQ(e.get_event_id(), MTM_1FILE_BAD_SEEMS_BINARY);
	}
}
