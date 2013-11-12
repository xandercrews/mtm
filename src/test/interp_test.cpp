#include "base/interp.h"
#include "gtest/gtest.h"

TEST(interp_test, null_fmt) {
	EXPECT_EQ("", interp(NULL));
	EXPECT_EQ("", interp(NULL, "hello"));
}

TEST(interp_test, empty_fmt) {
	EXPECT_EQ("", interp(""));
	EXPECT_EQ("", interp("", "hello"));
}

TEST(interp_test, no_argrefs_no_args) {
	EXPECT_EQ("hello", interp("hello"));
	EXPECT_EQ("hello", interp("hello"));
}

TEST(interp_test, no_argrefs_one_arg) {
	EXPECT_EQ("hello", interp("hello", 1));
	EXPECT_EQ("hello", interp("hello", "hi"));
}

TEST(interp_test, one_valid_argref) {
	EXPECT_EQ("hello 2", interp("hello %1", 2));
	EXPECT_EQ("hello 2 world", interp("hello %1 world", 2));
	EXPECT_EQ("hello cruel world", interp("hello %1{adjective} world",
			"cruel"));
	EXPECT_EQ("hello cruel2 world", interp("hello %12 world", "cruel"));
}

TEST(interp_test, hex_number) {
	EXPECT_EQ("hello 0x00fd", interp("hello 0x%1{%04x}", 253));
	EXPECT_EQ("hello 0x07C0B44", interp("hello 0x%1{%07X}", 8129348));
}

TEST(interp_test, padded_number) {
	EXPECT_EQ("003 hello", interp("%1{%03u} hello", 3));
}

TEST(interp_test, nine_numbers) {
	EXPECT_EQ("3 5 2 1 4 6 9 8 7", interp("%3 %5 %2 %1 %4 %6 %9 %8 %7",
			1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(interp_test, floating_point) {
	const double PI = 3.14159265358979;
	EXPECT_EQ("pi equals 3.14", interp("pi equals %1{%.2f}", PI));
	EXPECT_EQ("pi equals 3.14159265", interp("pi equals %1{%3.8f}", PI));
	EXPECT_EQ("pi equals 3.14e+00", interp("pi equals %1{%2.2e}", PI));
}
