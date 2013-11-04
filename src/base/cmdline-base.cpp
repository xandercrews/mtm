#include <string.h>

#include "base/cmdline-base.h"
#include "base/interp.h"

bool in_alternatives_str(char const * item, char const * alternatives) {
	if (item && *item && alternatives && *alternatives) {
		auto substr = strstr(alternatives, item);
		if (substr && (substr == alternatives || substr[-1] == '|')) {
			auto ender = alternatives[strlen(item)];
			return ender == '|' || ender == 0;
		}
	}
	return false;
}

bool CmdlineBase::is_flag(char const * arg) const {
	if (arg && *arg == '-') {
		return in_alternatives_str(arg, get_valid_flags());
	}
	return false;
}

bool CmdlineBase::is_option(char const * arg) const {
	if (arg && *arg == '-') {
		return in_alternatives_str(arg, get_valid_options());
	}
	return false;
}

bool matches_switch(char const * long_or_short_form, char const * long_form) {
	return long_form && long_or_short_form &&
		strcmp(long_or_short_form, long_form) == 0
			|| long_or_short_form[1] == long_form[2];
}

void CmdlineBase::add_error(std::string const & msg) {
	if (!errors.empty()) {
		errors += '\n';
	}
	errors += msg;
}

void CmdlineBase::parse(int argc, char const ** argv) {
	program_name = argc > 0 ? argv[0] : get_default_program_name();
	auto arg = strrchr(program_name, '/');
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
				add_error(interp("Expected %1 to be followed by a value.", arg));
			} else {
				auto value = argv[++i];
				if (matches_switch(arg, "--port")) {
					auto n = atoi(value);
					if (n < 1024 || n > 65536) {
						add_error(interp("Expected numeric port value > 1024 and < 65536 after %1{arg} -- not %2{value}.", arg, value));
					}
				}
				options.push_back(Option(arg, argv[++i]));
			}
		} else {
			positional_args.push_back(arg);
		}
	}
}

CmdlineBase::CmdlineBase(int argc, char const ** argv) {
	parse(argc, argv);
}

CmdlineBase::~CmdlineBase() {
}

bool CmdlineBase::has_flag(char const * full_name) const {
	if (full_name && *full_name == '-') {
		for (auto flag : flags) {
			if (matches_switch(flag, full_name)) {
				return true;
			}
		}
	}
	return false;
}

CmdlineBase::Options const & CmdlineBase::get_options() const {
	return options;
}

char const * CmdlineBase::get_option(char const * full_option_name) const {
	if (full_option_name && *full_option_name == '-') {
		for (auto Option : options) {
			if (matches_switch(Option.first, full_option_name)) {
				return Option.second;
			}
		}
	}
	return NULL;
}

CmdlineBase::Strings const & CmdlineBase::get_positional_args() const {
	return positional_args;
}

CmdlineBase::Strings const & CmdlineBase::get_flags() const {
	return flags;
}

std::string const & CmdlineBase::get_errors() const {
	return errors;
}

bool CmdlineBase::help_needed() const {
	return !get_errors().empty() || has_flag("--help");
}

char const * CmdlineBase::get_program_name() const {
	return program_name;
}

