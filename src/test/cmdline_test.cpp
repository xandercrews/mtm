/*
 * cmdline_test.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#include "domain/cmdline.h"
#include "gtest/gtest.h"

using nitro::Cmdline;

TEST(CmdlineTest, invalid_ctor_with_no_args) {
	Cmdline x(0, 0);
	ASSERT_FALSE(x.help_needed());
	EXPECT_STREQ("nitro", x.get_program_name());
}

TEST(CmdlineTest, help_needed_with_help_explicitly_requested) {
	char const * args[] = {"nitro", "--help"};
	Cmdline x(2, args);
	ASSERT_TRUE(x.help_needed());
}

TEST(CmdlineTest, help_needed_with_error) {
	char const * args[] = {"nitro", "--listenport"};
	Cmdline x(2, args);
	ASSERT_TRUE(x.help_needed());
	// Should get error about needing an argument for --listenport.
	ASSERT_TRUE(strstr(x.get_help().c_str(), "Expected --listenport to be followed by") != 0);
}

TEST(CmdlineTest, bogus_flag) {
	char const * args[] = {"nitro", "--bogus"};
	Cmdline x(2, args);
	ASSERT_TRUE(x.get_errors().empty());
	EXPECT_STREQ("--bogus", x.get_positional_args()[0]);
}

TEST(CmdlineTest, non_numeric_port_error) {
	char const * args[] = {"nitro", "--listenport", "abc"};
	Cmdline x(3, args);
	// Should get error about numeric port.
	ASSERT_TRUE(strstr(x.get_errors().c_str(), "Expected numeric port value") != 0);
}

TEST(CmdlineTest, out_of_range_port_error) {
	char const * args[] = {"nitro", "--publishport", "1234567"};
	Cmdline x(3, args);
	// Should get error about value out of range.
	ASSERT_TRUE(strstr(x.get_errors().c_str(), "6553"/*6 or 5*/) != 0);
}

TEST(CmdlineTest, program_name_from_argv_0) {
	char const * args[] = {"pickle", "--help"};
	Cmdline x(2, args);
	EXPECT_STREQ("pickle", x.get_program_name());
}

TEST(CmdlineTest, program_name_has_no_slashes) {
	char const * args[] = {"a/b/c"};
	Cmdline x(1, args);
	EXPECT_STREQ("c", x.get_program_name());
}
