#include <string.h>

#include "common/error.h"
#include "common/interp.h"

using std::string;
using std::runtime_error;

using namespace mtm::event;

namespace mtm {

static std::string make_msg(int eid, char const * source_fname,
		char const * source_func, unsigned source_line, MANY_ARGS_IMPL) {

	if (!source_fname || !*source_fname) {
		source_fname = "unknown file";
	}

	if (!source_func || !*source_func) {
		source_func = "unknown function";
	}

	Arg const * args[] = {&arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7,
			&arg8, &arg9};

	bool posix = event::get_component(eid) == kcPOSIX;
	bool interp_args = !posix;
	// Do some special handling if we try to render an external error.
	string msg = event::get_msg(eid);
	if (msg.empty()) {
		msg = "An external codebase returned an error/warning/info without an"
				" associated message.";
		interp_args = false;
	}

	string txt;
	if (interp_args) {
		interp_into(txt, msg.c_str(), args, 9);
	} else {
		int arg_count = 0;
		for (int i = 0; i < 9; ++i) {
			if (args[i] != &Arg::Empty) {
				if (!arg_count) {
					msg += " Args: ";
				} else {
					msg += ", ";
				}
				args[i]->append_to(msg);
			}
		}
		txt = msg;
	}

	// Make sure we don't display a fully-qualified path to source files --
	// we just want the part that's inside our project root.
	auto src = strstr(source_fname, "src/");
	if (src) {
		do {
			auto next = strstr(src + 4, "/src/");
			if (next) {
				src = next + 1;
			} else {
				break;
			}
		} while (true);
		if (src > source_fname && src[-1] == '/') {
			source_fname = src;
		}
	} else {
		src = strchr(source_fname, '/');
		if (src) {
			source_fname = src + 1;
		}
	}

	char fmt[128];
	sprintf(fmt, "%%1{sev} %s at %%3{fname}, %%4{func}(), line %%5: %%6{msg}",
			posix ? "%2{number}" : "0x%2{number:%08X}");
	string label = interp(fmt, get_severity_label(eid), eid, source_fname,
			source_func, source_line);

	// Insert label and return.
	txt.insert(0, label);
	return txt;
}

Error::Error(int eid, char const * source_fname, char const * source_func,
		unsigned source_line, MANY_ARGS_IMPL):
	runtime_error(make_msg(eid, source_fname, source_func,
			source_line, MANY_ARGS_LIST)),
	event_id(eid), source_fname(source_fname ? source_fname : ""),
	source_func(source_func ? source_func : ""), source_line(source_line) {
}

Error::~Error() {
}

int Error::get_event_id() const {
	return event_id;
}

char const * Error::get_source_fname() const {
	return source_fname;
}

char const * Error::get_source_func() const {
	return source_func;
}

unsigned Error::get_source_line() const {
	return source_line;
}

} /* namespace mtm */
