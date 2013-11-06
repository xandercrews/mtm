#include "base/guid.h"
#include "base/error.h"
#include "gtest/gtest.h"

TEST(guid_test, easy) {
	char buf[GUID_BUF_LEN];
	generate_guid(buf, sizeof(buf));
	ASSERT_EQ(GUID_BUF_LEN - 1, strlen(buf));
}

TEST(guid_test, too_short) {
	char buf[5];
	ASSERT_THROW(generate_guid(buf, sizeof(buf)), error_event);
}

TEST(guid_test, compare) {
	char const * a = "7d55c193-e82b-4076-aaed-2dc8bbc396ff";
	const char * A = "7D55C193-E82B-4076-AAED-2DC8BBC396FF";
	const char * a_with_noise = "{ 7d55.c193.e82b.4076.aaed.2dc8bbc396ff} ";
	const char * a_raw = "7d55c193e82b4076aaed2dc8bbc396ff";
	const char * b = "be5fed47ddc14f3aad2d8a59eea59b73";
	const char * b_with_garbage = "be5fed47ddc14f3aad2d8a59eea59b73pickle";

	EXPECT_EQ(0, compare_guids(NULL, NULL));
	EXPECT_EQ(1, compare_guids("", NULL));
	EXPECT_EQ(0, compare_guids("", ""));
	EXPECT_EQ(0, compare_guids(a, a));
	EXPECT_EQ(0, compare_guids(a, A));
	EXPECT_EQ(0, compare_guids(a, a_with_noise));
	EXPECT_EQ(0, compare_guids(A, a_raw));
	EXPECT_EQ(-1, compare_guids(A, b));
	EXPECT_EQ(1, compare_guids(b, a_raw));
	EXPECT_EQ(0, compare_guids(b, b_with_garbage));
}
