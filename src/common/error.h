#ifndef MTM_COMMON_ERROR_H_
#define MTM_COMMON_ERROR_H_

#include <stdexcept>
#include "common/event.h"
#include "common/interp.h"

using mtm::event::EID;

namespace mtm {

#define MTM_ERROR(eid, ...) \
	Error(mtm::event::eid, __FILE__, __func__, __LINE__, ##__VA_ARGS__)

/**
 * A class for all exceptions that MTM emits.
 */
class Error: public std::runtime_error {
public:
	virtual ~Error();
	/**
	 * Create an error with an event, plus an arbitrary number of args of any
	 * data type.
	 *
	 * It's expected that constructors will usually be called via the
	 * MTM_ERROR macro, instead of directly, since that automates usage of the
	 * __FILE__ and __LINE__ values exposed by the preprocessor.
	 */
	Error(int eid, char const * source_fname, char const * func,
			unsigned source_line, MANY_OARGS);

	// I did not implement any of the optimized overloads for interp-style
	// functions, because it keeps code cleaner. Exceptions are by definition
	// exceptional--they are not a "common case" that we have to make fast.

	// Let code lookup information about the error.
	int get_event_id() const;
	char const * get_source_fname() const;
	char const * get_source_func() const;
	unsigned get_source_line() const;

private:
	int event_id;
	char const * source_fname;
	char const * source_func;
	unsigned source_line;
};


} /* namespace mtm */

#endif /* MTM_COMMON_ERROR_H_ */
