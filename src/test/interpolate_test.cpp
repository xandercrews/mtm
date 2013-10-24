#include <limits.h>

#include "common/interpolate.h"
#include "gtest/gtest.h"

// First test
TEST(InterpolateTest, OneArg) {
  EXPECT_EQ("hello", interpolate("hello", 1));
}
