/*
 * cmdline.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#include <string.h>

#include "common/cmdline.h"
#include "common/interpolate.h"

namespace mtm {

inline bool in_alternatives_str(char const * item, char const * alternatives) {
	auto substr = strstr(alternatives, item);
	if (substr && (substr == alternatives || substr[-1] == '|')) {
		auto ender = alternatives[strlen(item)];
		return ender == '|' || ender == 0;
	}
	return false;
}

char const * const VALID_FLAGS = "--help|-h|--slave|-s";
inline bool is_flag(char const * arg) {
	if (arg && *arg == '-') {
		return in_alternatives_str(arg, VALID_FLAGS);
	}
	return false;
}

char const * const VALID_OPTIONS = "--port|-p";
inline bool is_option(char const * arg) {
	if (arg && *arg == '-') {
		return in_alternatives_str(arg, VALID_OPTIONS);
	}
	return false;
}

inline bool matches_switch(char const * long_or_short_form, char const * long_form) {
	return strcmp(long_or_short_form, long_form) == 0 || long_or_short_form[1] == long_form[2];
}

struct Cmdline::Data {
	char const * program_name;
	Cmdline::Options options;
	Cmdline::Strings positional_args;
	Cmdline::Strings flags;
	std::string errors;

	inline void add_error(std::string const msg) {
		if (!errors.empty()) {
			errors += '\n';
		}
		errors += msg;
	}

	inline void parse(int argc, char const ** argv) {
		program_name = argc > 0 ? argv[0] : "mtm";
		for (auto i = 1; i < argc; ++i) {
			auto arg = argv[i];
			if (is_flag(arg)) {
				flags.push_back(arg);
			} else if (is_option(arg)) {
				if (i >= argc - 1) {
					add_error(interpolate("Expected %1 to be followed by a value.", arg));
				} else {
					auto value = argv[++i];
					if (matches_switch(arg, "--port")) {
						auto n = atoi(value);
						if (n < 1024 || n > 65536) {
							add_error(interpolate("Expected numeric port value > 1024 and < 65536 after %1{arg} -- not %2{value}.", arg, value));
						}
					}
					options.push_back(Option(arg, argv[++i]));
				}
			} else {
				positional_args.push_back(arg);
			}
		}
	}
};



Cmdline::Cmdline(int argc, char const ** argv) : data(new Data) {
	data->parse(argc, argv);
}

Cmdline::~Cmdline() {
	delete data;
}

bool Cmdline::has_flag(char const * full_name) const {
	if (full_name && *full_name == '-') {
		for (auto flag : data->flags) {
			if (matches_switch(flag, full_name)) {
				return true;
			}
		}
	}
	return false;
}

Cmdline::Options const & Cmdline::get_options() const {
	return data->options;
}

Cmdline::Strings const & Cmdline::get_positional_args() const {
	return data->positional_args;
}

Cmdline::Strings const & Cmdline::get_flags() const {
	return data->flags;
}

std::string const & Cmdline::get_errors() const {
	return data->errors;
}

bool Cmdline::help_needed() const {
	return !get_errors().empty() || has_flag("--help");
}

char const * Cmdline::get_program_name() const {
	return data->program_name;
}

std::string Cmdline::get_help() const {
	return interpolate(
			"%1{errors}\n%2{progname} -- run a batch of similar jobs at high speed\n"
			"\n"
			"  Syntax: %2{progname} [flags] [options] [batch file(s)]\n"
			"\n"
			"    Flags:\n"
			"      --slave or -s -- Await instructions for a different %2{progname} instance.\n"
			"      --help or -h  -- Display this screen.\n"
			"\n"
			"    Options:\n"
			"      --port or -p  -- Override listen port (%3{defport} is used by default).\n"
			"\n",
			data->errors, get_program_name(), DEFAULT_PORT
		);
}

} /* namespace mtm */
