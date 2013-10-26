#ifndef TEST_UTIL_H_
#define TEST_UTIL_H_

#include <string.h>

#include "gtest/gtest.h"

void expect_str_contains(char const * haystack, char const * needle,
		bool expected = true);

#endif /* TEST_UTIL_ */
