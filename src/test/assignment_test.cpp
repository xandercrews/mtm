#include "base/error.h"
#include "base/event_codes.h"

#include "domain/assignment.h"
#include "domain/event_codes.h"

#include "gtest/gtest.h"

#include "test/test_util.h"

using namespace base::event_codes;
using namespace nitro::event_codes;
using namespace nitro;

TEST(assignment_test, bad_id) {
	assignment a(nullptr);
	// Should not crash. We should get a random guid assigned as id.
	ASSERT_EQ(36, strlen(a.get_id()));
}

TEST(assignment_test, get_request_msg) {
	assignment a("test");
	// Add a few tasks.
	a.add_task(1, "qsub task 1");
	a.add_task(2, "qsub task 2");
	auto txt = a.get_request_msg();
	expect_str_contains(txt, events::get_std_id_repr(NITRO_HERE_IS_ASSIGNMENT));
	expect_str_contains(txt, "\"1\"");
	expect_str_contains(txt, "\"qsub task 2\"");
}

TEST(assignment_test, get_status_msg) {
	assignment a("test");
	// Add a few tasks.
	a.add_task(1, "qsub task 1");
	a.add_task(2, "qsub task 2");

	auto txt = a.get_status_msg();
	expect_str_contains(txt,
			events::get_std_id_repr(NITRO_ASSIGNMENT_PROGRESS_REPORT));
	expect_str_contains(txt, "done_count\" : 0");
	expect_str_contains(txt, "pending\" : [ \"1\", \"2\" ]");

	a.complete_task(2);
	txt = a.get_status_msg();
	expect_str_contains(txt, "done_count\" : 1");
	expect_str_contains(txt, "done\" : [ \"2\" ]");

}
