/*
 * batch_test.cpp
 *
 *  Created on: Oct 25, 2013
 *      Author: dhardman
 */

#include <stdlib.h>

#include "gtest/gtest.h"
#include "common/batch.h"
#include "common/error.h"

using namespace mtm;
using namespace mtm::event;

TEST(BatchTest, nonexistent_file) {
	Batch b("doesn't@exist");
	ASSERT_EQ(NULL, b.next_line());
	ASSERT_DOUBLE_EQ(1.0, b.ratio_complete());
}

TEST(BatchTest, binary_file) {
#if 0
	try {
		Batch b("proc/self/exe");
		ADD_FAILURE() << "Expected error about binary file.";
	} catch (Error const & e) {
		ASSERT_EQ(e.get_event_id(), MTM_1FILE_BAD_SEEMS_BINARY);
	}
#endif
}

TEST(BatchTest, text_file) {
#if 0
	// First, create a temporary txt file.
	char buf[512];
	strcpy(buf, "/tmp/batch_test_XXXXXX");
	int handle = mkstemp(buf);
	sprintf(buf, "/proc/self/fd/%d", handle);
	ssize_t bytes_copied = readlink(buf, buf, sizeof(buf));
	close(handle);
	if (bytes_copied > 0) {
		buf[bytes_copied] = 0;
	} else {
		FAIL() << "Couldn't create temp file.";
	}

	// Guarantee that the temp file is deleted no matter how we exit scope.
	struct FileCleanup {
		std::string fname;
		FileCleanup(char const * fname) : fname(fname) {}
		~FileCleanup() {
			unlink(fname.c_str());
		}
	} fc(buf);

	FILE * f = fopen(buf, "w");
	buf[0] = 0;
	char const * LINE = "Here's a happy line";
	for (int i = 0; i < 5; ++i) {
		switch (i) {
		case 0:
			strcat(buf, "  "); break;
		case 1:
			strcat(buf, "\n\n\n"); break;
		case 2:
			strcat(buf, "\r\n\t"); break;
		case 3:
			strcat(buf, "  #"); break;
		}
		strcat(buf, LINE);
		if (i == 4) {
			strcat(buf, "  ");
		}
		strcat(buf, "\n");
	}
	fwrite(buf, 1, strlen(buf), f);
	fclose(f);

	// Now see if we get exactly 4 identical lines. (The 5th line that we wrote
	// was preceded by a comment and should be skipped.)
	Batch b(fc.fname.c_str());
	for (int i = 0; i < 4; ++i) {
		char const * line = b.next_line();
		ASSERT_STREQ(LINE, line);
	}
	ASSERT_STREQ(NULL, b.next_line());
#endif
}
