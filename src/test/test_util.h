#ifndef _TEST_TEST_UTIL_H_
#define _TEST_TEST_UTIL_H_

#include <string>
#include <string.h>

#include "gtest/gtest.h"

void expect_str_contains(char const * haystack, char const * needle,
		bool expected = true);

void expect_str_contains(std::string const & haystack, char const * needle,
		bool expected = true);

void expect_str_contains(char const * haystack, std::string const & needle,
		bool expected = true);

#endif // sentry
