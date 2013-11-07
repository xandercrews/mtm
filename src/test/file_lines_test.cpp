#include "base/error.h"
#include "base/event_ids.h"
#include "base/file_lines.h"

#include "gtest/gtest.h"

using namespace base::event_ids;

TEST(batch_test, nonexistent_file) {
	try {
		file_lines fl("doesn't@exist");
		ADD_FAILURE() << "Expected error_event about unavailable file.";
	} catch (error_event const & e) {
		ASSERT_EQ(e.get_event_id(), E_INPUT_FILE_1PATH_UNREADABLE);
	}
}

TEST(batch_test, readable_binary_file) {
	try {
		file_lines fl("/usr/bin/ls");
		ADD_FAILURE() << "Expected error_event about binary file.";
	} catch (error_event const & e) {
		ASSERT_EQ(e.get_event_id(), E_1FILE_BAD_SEEMS_BINARY);
	}
}

TEST(batch_test, empty_existing_file) {
	try {
		file_lines fl("/tmp/");
		ADD_FAILURE() << "Expected error_event about unreadable file.";
	} catch (error_event const & e) {
		ASSERT_EQ(e.get_event_id(), E_INPUT_FILE_1PATH_EMPTY);
	}
}

TEST(batch_test, text_file) {
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
	file_lines fl(fc.fname.c_str());
	for (int i = 0; i < 4; ++i) {
		char const * line = fl.next();
		ASSERT_STREQ(LINE, line);
	}
	ASSERT_STREQ(NULL, fl.next());
}
