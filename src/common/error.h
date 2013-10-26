#ifndef MTM_COMMON_ERROR_H_
#define MTM_COMMON_ERROR_H_

#include <stdexcept>
#include "common/event.h"
#include "common/interp.h"

using mtm::event::EID;

namespace mtm {

#define MTM_ERROR(eid, ...) \
	Error(mtm::event::eid, __FILE__, __LINE__, ##__VA_ARGS__)

/**
 * A class for all exceptions that MTM emits.
 */
class Error: public std::runtime_error {
public:
	/**
	 * Create an error with an event, plus an arbitrary number of args of any
	 * data type.
	 *
	 * It's expected that constructors will usually be called via the
	 * MTM_ERROR macro, instead of directly, since that automates usage of the
	 * __FILE__ and __LINE__ values exposed by the preprocessor.
	 */
	Error(EID error, char const * source_fname, int source_line, MANY_OARGS);

	// I did not implement any of the optimized overloads for interp-style
	// functions, because it keeps code cleaner. Exceptions are by definition
	// exceptional--they are not a "common case" that we have to make fast.

	virtual ~Error();
};


} /* namespace mtm */

#endif /* MTM_COMMON_ERROR_H_ */
