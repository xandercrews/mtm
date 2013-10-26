/*
 * Error.cpp
 *
 *  Created on: Oct 25, 2013
 *      Author: dhardman
 */

#include "common/error.h"
#include "common/interp.h"

using std::string;
using std::runtime_error;

using mtm::event::EID;

namespace mtm {

static std::string make_msg(EID error, char const * source_fname,
		int source_line, MANY_ARGS_IMPL) {

	// First, expand the text of the error message.
	string txt;
	Arg const * args[] = {&arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7,
			&arg8, &arg9};
	interp_into(txt, event::get_msg(error), args, 9);

	// Now build an appropriate label as a prefix for the message text.
	char const * const fmt =
			"%1{severity} 0x%2{number:%08d} at %3{fname}, line %4: %5{msg}";
	string label = interp(fmt,
			event::SeverityLabels[event::get_severity(error)],
			static_cast<int>(error), source_fname, source_line);

	// Insert label and return.
	txt.insert(0, label);
	return txt;
}

Error::Error(EID error, char const * source_fname, int source_line,
		MANY_ARGS_IMPL):
	runtime_error(make_msg(error, source_fname, source_line, MANY_ARGS_LIST)),
	event_id(error) {
}

Error::~Error() {
}

EID Error::get_event_id() const {
	return event_id;
}

} /* namespace mtm */
