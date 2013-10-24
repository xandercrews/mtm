#include "common/interpolate.h"
#include "gtest/gtest.h"

TEST(InterpolateTest, no_argrefs_one_arg) {
  EXPECT_EQ("hello", interpolate("hello", 1));
}

TEST(InterpolateTest, one_valid_argref) {
  EXPECT_EQ("hello 2", interpolate("hello %1", 2));
}
