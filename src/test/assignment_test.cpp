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
	a.ready_task(1, "qsub task 1");
	a.ready_task(2, "qsub task 2");
	auto txt = a.get_request_msg();
	expect_str_contains(txt, events::get_std_id_repr(NITRO_HERE_IS_ASSIGNMENT));
	expect_str_contains(txt, "\"1\"");
	expect_str_contains(txt, "\"qsub task 2\"");
}

TEST(assignment_test, get_status_msg) {
	assignment a("test");
	// Add a few tasks.
	a.ready_task(1, "qsub task 1");
	a.ready_task(2, "qsub task 2");

	auto txt = a.get_status_msg();
	expect_str_contains(txt,
			events::get_std_id_repr(NITRO_ASSIGNMENT_PROGRESS_REPORT));
	expect_str_contains(txt, "complete_count\" : 0");
	expect_str_contains(txt, "active_count\" : 0");
	expect_str_contains(txt, "ready\" : [ \"1\", \"2\" ]");

	a.activate_task(1);
	a.activate_task(2);
	a.complete_task(2);
	txt = a.get_status_msg();
	expect_str_contains(txt, "ready_count\" : 0");
	expect_str_contains(txt, "active_count\" : 1");
	expect_str_contains(txt, "active\" : [ \"1\" ]");
	expect_str_contains(txt, "complete_count\" : 1");
	expect_str_contains(txt, "complete\" : [ \"2\" ]");
}

TEST(assignment_test, fill_from_lines) {
	assignment asgn("a1", "qsub line 1\nqsub line 2\nqsub line 3");
	assignment::tasklist_t const & readylist = asgn.get_list_by_status(
			ts_ready);
	EXPECT_EQ(3, readylist.size());
	for (auto i = readylist.cbegin(); i != readylist.cend(); ++i) {
		auto cmd = (*i)->get_cmdline();
		expect_str_contains(cmd, "qsub line ");
		EXPECT_TRUE(strchr(cmd, '\n') == 0);
	}
}
