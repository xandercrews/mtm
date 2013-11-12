#ifndef _BASE_ERROR_H_
#define _BASE_ERROR_H_

#include <stdexcept>
#include "base/events.h"
#include "base/interp.h"

#define ERROR_EVENT(eid, ...) \
	error_event(eid, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

/**
 * A class for all exceptions that MTM emits.
 */
class error_event: public std::runtime_error {
public:
	virtual ~error_event() noexcept;
	/**
	 * Create an error with an event, plus an arbitrary number of args of any
	 * data type.
	 *
	 * It's expected that constructors will usually be called via the
	 * NITRO_ERROR macro, instead of directly, since that automates usage of the
	 * __FILE__ and __LINE__ values exposed by the preprocessor.
	 */
	error_event(int eid, char const * source_fname, char const * func,
			unsigned source_line, MANY_OARGS);

	// I did not implement any of the optimized overloads for interp-style
	// functions, because it keeps code cleaner. Exceptions are by definition
	// exceptional--they are not a "common case" that we have to make fast.

	// Allow other code to lookup information about the error.
	int get_event_code() const;
	char const * get_source_fname() const;
	char const * get_source_func() const;
	unsigned get_source_line() const;

private:
	int event_id;
	char const * source_fname;
	char const * source_func;
	unsigned source_line;
};

#endif // sentry
