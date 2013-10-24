/*
 * cmdline_test.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#include "common/cmdline.h"
#include "gtest/gtest.h"

using namespace mtm;

TEST(CmdlineTest, invalid_ctor_with_no_args) {
	Cmdline x(0, 0);
	ASSERT_FALSE(x.help_needed());
	EXPECT_STREQ("mtm", x.get_program_name());
}

TEST(CmdlineTest, help_needed_with_help_explicitly_requested) {
	char const * args[] = {"mtm", "--help"};
	Cmdline x(2, args);
	ASSERT_TRUE(x.help_needed());
}

TEST(CmdlineTest, help_needed_with_error) {
	char const * args[] = {"mtm", "--port"};
	Cmdline x(2, args);
	ASSERT_TRUE(x.help_needed());
}

TEST(CmdlineTest, program_name_from_argv_0) {
	char const * args[] = {"pickle", "--help"};
	Cmdline x(2, args);
	EXPECT_STREQ("pickle", x.get_program_name());
}
