#include "base/dbc.h"
#include "base/event_codes.h"

#include "gtest/gtest.h"

#include "test/test_util.h"

using namespace base::event_codes;

TEST(dbc_test, pre_and_check_satisfied) {
	// Should not throw.
	PRECONDITION(true);
	CHECK(true);
}

TEST(dbc_test, post_satisfied) {
	int i = 2;
	{
		int j = 3;
		// If our postconditions are working right, we test the state when the
		// *scope* exits, not when we write the line, and not necessarily at the
		// exit of the *function*.
		POSTCONDITION(i > j);
		i = 4; // this line should satisfy the postcondition.
	}
	i = 1; // this line should run after the postcondition is satisfied.
}

TEST(dbc_test, failed_precondition) {
	EXPECT_THROW_WITH_CODE(PRECONDITION(nullptr),
			E_PRECONDITION_1EXPR_VIOLATED);
}

TEST(dbc_test, failed_check) {
	EXPECT_THROW_WITH_CODE(CHECK(
			"this is a sentence that ends in a period" && false),
			E_CHECK_1EXPR_VIOLATED);
}

#if 0
TEST(dbc_test, failed_postcondition) {
	try {
		int i = 2;
		int j = 3;
		POSTCONDITION(i > j);
		ADD_FAILURE() << "Expected POSTCONDITION to fail.";
	} catch (error_event const & e) {
		EXPECT_EQ(E_POSTCONDITION_1EXPR_VIOLATED, e.get_event_code());
	}
}
#endif
