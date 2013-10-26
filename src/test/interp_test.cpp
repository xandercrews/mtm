#include "common/interp.h"
#include "gtest/gtest.h"

TEST(InterpTest, null_fmt) {
  EXPECT_EQ("", interp(NULL));
  EXPECT_EQ("", interp(NULL, "hello"));
}

TEST(InterpTest, empty_fmt) {
  EXPECT_EQ("", interp(""));
  EXPECT_EQ("", interp("", "hello"));
}

TEST(InterpTest, no_argrefs_no_args) {
  EXPECT_EQ("hello", interp("hello"));
  EXPECT_EQ("hello", interp("hello"));
}

TEST(InterpTest, no_argrefs_one_arg) {
  EXPECT_EQ("hello", interp("hello", 1));
  EXPECT_EQ("hello", interp("hello", "hi"));
}

TEST(InterpTest, one_valid_argref) {
  EXPECT_EQ("hello 2", interp("hello %1", 2));
  EXPECT_EQ("hello 2 world", interp("hello %1 world", 2));
  EXPECT_EQ("hello cruel world", interp("hello %1{adjective} world", "cruel"));
  EXPECT_EQ("hello cruel2 world", interp("hello %12 world", "cruel"));
}
