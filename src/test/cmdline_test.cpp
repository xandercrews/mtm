#include "domain/cmdline.h"
#include "gtest/gtest.h"
#include "test/test_util.h"

using nitro::cmdline;

void expect_errors_contain(cmdline_base::errors_t const & errors, char const * txt) {
	for (auto e: errors) {
		if (strstr(e.what(), txt)) {
			return;
		}
	}
	ADD_FAILURE() << "Expected to see \"" << txt << "\" in a cmdline error ("
			<< errors.size() << " errors reported).";
}

TEST(cmdline_test, invalid_ctor_with_no_args) {
	cmdline x(0, 0);
	ASSERT_FALSE(x.help_needed());
	EXPECT_STREQ("nitro", x.get_program_name());
}

TEST(cmdline_test, help_needed_with_help_explicitly_requested) {
	char const * args[] = {"nitro", "--help"};
	cmdline x(2, args);
	ASSERT_TRUE(x.help_needed());
}

TEST(cmdline_test, help_needed_with_error) {
	char const * args[] = {"nitro", "--replyport"};
	cmdline x(2, args);
	ASSERT_TRUE(x.help_needed());
	// Should get error about needing an argument for --replyport.
	expect_errors_contain(x.get_errors(), "Expected --replyport option to be followed by");
}

TEST(cmdline_test, bogus_flag) {
	char const * args[] = {"nitro", "--bogus"};
	cmdline x(2, args);
	expect_errors_contain(x.get_errors(), "--bogus");
}

TEST(cmdline_test, non_numeric_port_error) {
	char const * args[] = {"nitro", "--replyport", "abc"};
	cmdline x(3, args);
	// Should get error about numeric port.
	expect_errors_contain(x.get_errors(), "Expected numeric port value");
}

TEST(cmdline_test, out_of_range_port_error) {
	char const * args[] = {"nitro", "--publishport", "1234567"};
	cmdline x(3, args);
	// Should get error about value out of range.
	expect_errors_contain(x.get_errors(), "6553"/*6 or 5*/);
}

TEST(cmdline_test, program_name_from_argv_0) {
	char const * args[] = {"pickle", "--help"};
	cmdline x(2, args);
	EXPECT_STREQ("pickle", x.get_program_name());
}

TEST(cmdline_test, program_name_has_no_slashes) {
	char const * args[] = {"a/b/c"};
	cmdline x(1, args);
	EXPECT_STREQ("c", x.get_program_name());
}
