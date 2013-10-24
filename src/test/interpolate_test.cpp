#include "common/interpolate.h"
#include "gtest/gtest.h"

TEST(InterpolateTest, no_argrefs_no_args) {
  EXPECT_EQ("hello", interpolate("hello"));
  EXPECT_EQ("hello", interpolate("hello"));
}

TEST(InterpolateTest, no_argrefs_one_arg) {
  EXPECT_EQ("hello", interpolate("hello", 1));
  EXPECT_EQ("hello", interpolate("hello", "hi"));
}

TEST(InterpolateTest, one_valid_argref) {
  EXPECT_EQ("hello 2", interpolate("hello %1", 2));
  EXPECT_EQ("hello 2 world", interpolate("hello %1 world", 2));
  EXPECT_EQ("hello cruel world", interpolate("hello %1{adjective} world", "cruel"));
}
