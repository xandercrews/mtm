#include <string.h>

#include "base/cmdline.h"
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

bool Cmdline::is_flag(char const * arg) const {
	if (arg && *arg == '-') {
		return in_alternatives_str(arg, get_valid_flags());
	}
	return false;
}

bool Cmdline::is_option(char const * arg) const {
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

void Cmdline::add_error(std::string const & msg) {
	if (!errors.empty()) {
		errors += '\n';
	}
	errors += msg;
}

void Cmdline::parse(int argc, char const ** argv) {
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

Cmdline::Cmdline(int argc, char const ** argv) {
	parse(argc, argv);
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

char const * Cmdline::get_option(char const * full_option_name) const {
	if (full_option_name && *full_option_name == '-') {
		for (auto Option : data->options) {
			if (matches_switch(Option.first, full_option_name)) {
				return Option.second;
			}
		}
	}
	return NULL;
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
	return interp(
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

