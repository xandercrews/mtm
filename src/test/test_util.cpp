#include "test/test_util.h"

void expect_str_contains(char const * haystack, char const * needle,
		bool expected) {
	if ((strstr(haystack, needle) != 0) != expected) {
		char const * in_phrase = expected ? "IN" : "NOT IN";
		ADD_FAILURE() << "Expected \"" << needle << "\" " << in_phrase
				<< " \"" << haystack << "\".";
	}
}
