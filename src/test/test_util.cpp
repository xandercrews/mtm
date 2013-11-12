#include "test/test_util.h"

void expect_str_contains(char const * haystack, char const * needle,
		bool expected) {
	if ((strstr(haystack, needle) != 0) != expected) {
		char const * in_phrase = expected ? "IN" : "NOT IN";
		ADD_FAILURE() << "Expected \"" << needle << "\" " << in_phrase
				<< " \"" << haystack << "\".";
	}
}

void expect_str_contains(char const * haystack, std::string const & needle,
		bool expected) {
	expect_str_contains(haystack, needle.c_str(), expected);
}

void expect_str_contains(std::string const & haystack, char const * needle,
		bool expected) {
	expect_str_contains(haystack.c_str(), needle, expected);
}

std::string make_temp_file() {
	char buf[512];
	strcpy(buf, "/tmp/test_XXXXXX");
	int handle = mkstemp(buf);
	sprintf(buf, "/proc/self/fd/%d", handle);
	ssize_t bytes_copied = readlink(buf, buf, sizeof(buf));
	close(handle);
	if (bytes_copied > 0) {
		buf[bytes_copied] = 0;
	} else {
		ADD_FAILURE() << "Couldn't create temp file.";
	}
	return buf;
}
