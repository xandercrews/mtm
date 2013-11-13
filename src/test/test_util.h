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

void expect_str_contains(std::string const & haystack,
		std::string const & needle, bool expected = true);

struct FileCleanup {
	std::string fname;
	FileCleanup(char const * fname) : fname(fname) {}
	~FileCleanup() {
		unlink(fname.c_str());
	}
};

std::string make_temp_file();

#define EXPECT_THROW_WITH_CODE(block, code) \
try { \
	block; \
	ADD_FAILURE() << "Expected block to throw an error_event."; \
} catch (error_event const & e) { \
	EXPECT_EQ(code, e.get_event_code()); \
}


#endif // sentry
