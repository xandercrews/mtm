#ifndef _BASE_DBC_H_
#define _BASE_DBC_H_

#include <boost/scope_exit.hpp>

#include "base/error.h"
#include "base/event_codes.h"

/**
 * Provide support for "design by contract", which allows functions to declare
 * preconditions and postconditions, and guarantee that they're enforced.
 *
 * DBC is similar in concept to assertions, but it is designed to be used in
 * release builds, and it's a bit more helpful about reporting what goes
 * wrong in a way that the application can handle flexibly. See
 * http://www.eiffel.com/developers/design_by_contract.html for DBC theory.
 */

/**
 * Define what should happen when a contract is violated. By default, we
 * throw an exception. Other possibilities might include calling exit(1) with
 * a message on stderr, emitting a special message to the message queue, etc.
 * Ignoring contract violations is a recipe for disaster and should NEVER be
 * the choice, because every contract violation is a coding error that causes
 * downstream code to produce undefined results. Better to bail out as soon as
 * we recognize a problem, rather than let a blind maniac pilot the plane and
 * hope we land without hurting ourselves or others.
 */
#ifndef CONTRACT_VIOLATION_ACTION
#define CONTRACT_VIOLATION_ACTION(code, expr_text) \
	throw ERROR_EVENT(code, expr_text)
#endif

/**
 * A precondition is a constraint that callers must obey when they invoke a
 * function, such as requiring that a pointer not be NULL. Declare them at the
 * top of the code block of the called function.
 */
#define PRECONDITION(expr) if (!(expr)) \
	CONTRACT_VIOLATION_ACTION(\
			base::event_codes::E_PRECONDITION_1EXPR_VIOLATED, #expr)

/**
 * A check is a way for paranoid callers to verify the behavior of called
 * functions. Declare them right after a function call that you don't trust.
 */
#define CHECK(expr) if (!(expr)) \
	CONTRACT_VIOLATION_ACTION(\
			base::event_codes::E_CHECK_1EXPR_VIOLATED, #expr)

/**
 * A postcondition is a guarantee that a function makes on exit. When
 * postconditions fail, it is a bug in the function, not a bug in the caller
 * or a problem in any of the callees. Naively, we'd declare postconditions in
 * the ...finally portion of a try...finally block that encloses a full
 * function body, or an interesting subset thereof. But C++ doesn't support
 * try...finally. In C++03, that didn't leave us very happy... but in C++ 11,
 * we have lambdas. And magic ensues... The following code allows you to
 * declare a postcondition as soon as all the variables you need to test are
 * in scope, and still have those variables evaluated *whenever the function
 * goes out of scope*. See unit tests for typical usage, or see the boost
 * documentation at http://www.boost.org/doc/libs/1_54_0/libs/scope_exit/doc/html.
 *
 */
#define POSTCONDITION(expr) \
	BOOST_SCOPE_EXIT_ALL(&) { \
		if (!(expr)) { \
			CONTRACT_VIOLATION_ACTION( \
					base::event_codes::E_POSTCONDITION_1EXPR_VIOLATED, #expr); \
		} \
	}

#endif // sentry
