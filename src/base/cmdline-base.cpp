#include <stdlib.h>
#include <string.h>

#include "base/cmdline-base.h"
#include "base/interp.h"
#include "base/event_codes.h"

using namespace base::event_codes;

bool in_alternatives_str(char const * item, char const * alternatives) {
	if (item && *item && alternatives && *alternatives) {
		auto substr = strstr(alternatives, item);
		if (substr && (substr == alternatives || substr[-1] == '|')) {
			auto ender = substr[strlen(item)];
			return ender == '|' || ender == 0;
		}
	}
	return false;
}

bool cmdline_base::is_flag(char const * arg) const {
	if (arg && *arg == '-') {
		return in_alternatives_str(arg, get_valid_flags());
	}
	return false;
}

bool cmdline_base::is_option(char const * arg) const {
	if (arg && *arg == '-') {
		return in_alternatives_str(arg, get_valid_options());
	}
	return false;
}

bool matches_switch(char const * long_or_short_form, char const * long_form) {
	return long_form && long_or_short_form &&
		(strcmp(long_or_short_form, long_form) == 0
			|| long_or_short_form[1] == long_form[2]);
}

void cmdline_base::add_error(error_event const & err) {
	errors.push_back(err);
}

void cmdline_base::parse(int argc, char const ** argv) {
	program_name = argc > 0 ? argv[0] : get_default_program_name();
	char const * arg = strrchr(program_name, '/');
	// As long as the program_name doesn't end in a slash, trim
	// off the path portion.
	if (arg && arg[1]) {
		program_name = arg+1;
	}
	for (auto i = 1; i < argc; ++i) {
		arg = argv[i];
		if (is_flag(arg)) {
			flags.push_back(arg);
		} else if (is_option(arg)) {
			if (i >= argc - 1) {
				add_error(ERROR_EVENT(
						E_CMDLILNE_MISSING_VALUE_FOR_1OPTION, arg));
			} else {
				options.push_back(option_t(arg, argv[++i]));
			}
		} else {
			if (arg[0] == '-') {
				add_error(ERROR_EVENT(
						E_CMDLINE_UNRECOGNIZED_1SWITCH, arg));
			} else {
				positional_args.push_back(arg);
			}
		}
	}
}

cmdline_base::cmdline_base() {
}

cmdline_base::~cmdline_base() {
}

bool cmdline_base::has_flag(char const * full_name) const {
	if (full_name && *full_name == '-') {
		for (auto flag : flags) {
			if (matches_switch(flag, full_name)) {
				return true;
			}
		}
	}
	return false;
}

cmdline_base::options_t const & cmdline_base::get_options() const {
	return options;
}

char const * cmdline_base::get_option(char const * full_option_name,
		char const * default_value) const {
	if (full_option_name && *full_option_name == '-') {
		for (auto Option : options) {
			if (matches_switch(Option.first, full_option_name)) {
				return Option.second;
			}
		}
	}
	return default_value;
}

int cmdline_base::get_option_as_int(char const * full_option_name, int bad)
		const {
	auto val = get_option(full_option_name);
	if (val) {
		char * last_digit;
		auto end = strchr(val, 0);
		int n = strtol(val, &last_digit, 10);
		if (last_digit == end) {
			return n;
		}
	}
	return bad;
}

cmdline_base::strings_t const & cmdline_base::get_positional_args() const {
	return positional_args;
}

cmdline_base::strings_t const & cmdline_base::get_flags() const {
	return flags;
}

cmdline_base::errors_t const & cmdline_base::get_errors() const {
	return errors;
}

bool cmdline_base::help_needed() const {
	return !get_errors().empty() || has_flag("--help");
}

char const * cmdline_base::get_program_name() const {
	return program_name;
}

