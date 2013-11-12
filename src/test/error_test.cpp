#include <errno.h>

#include "gtest/gtest.h"
#include "base/error.h"
#include "base/event_codes.h"
#include "domain/event_codes.h"
#include "test/test_util.h"

using namespace base::event_codes;
using namespace nitro::event_codes;

TEST(error_test, simple) {
	auto n = __LINE__; auto e = ERROR_EVENT(E_NOT_IMPLEMENTED);
	auto msg = e.what();
	expect_str_contains(msg, __FILE__);
	expect_str_contains(msg, __func__);
	char buf[20];
	sprintf(buf, "%d", n);
	expect_str_contains(msg, "0x2001C001");
	expect_str_contains(msg, buf);
	expect_str_contains(msg, "not yet been implemented");
}

TEST(error_test, posix_ctor) {
	auto e = error_event(ENOENT, "gloop", "func", 99);
	auto msg = e.what();
	expect_str_contains(msg, "gloop");
	expect_str_contains(msg, "func()");
	// All posix errors have error severity, even though they don't have
	// any bits set in the severity range.
	expect_str_contains(msg, "rror 2");
	expect_str_contains(msg, "Info", false);
	// Posix errors aren't supposed to display in hex.
	expect_str_contains(msg, "0x", false);
	// Make sure we got some text from the posix subsystem.
	expect_str_contains(msg, "file or directory");
}

TEST(error_test, null_fname_and_func) {
	// These two lines should not crash.
	auto e = error_event(E_NOT_IMPLEMENTED, NULL, NULL, 3);
	e.what();
}
