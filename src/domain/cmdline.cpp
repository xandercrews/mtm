/*
 * cmdline.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#include <string.h>

#include "domain/cmdline.h"

namespace nitro {

char const * Cmdline::get_valid_flags() const {
	return "--help|-h|--slave|-s";
}

char const * Cmdline::get_valid_options() const {
	return "--port|-p";
}

char const * Cmdline::get_default_program_name() const {
	return "nitro";
}

void Cmdline::parse(int argc, char const ** argv) {
	::Cmdline::parse(argc, argv);
	char const * port = get_option("--port");
	if (port) {
		auto end = strchr(value, 0);
		auto last_digit = end;
		auto n = strtol(value, &last_digit, 10);
		if (n < 1024 || n > 65536 || last_digit != end) {
			add_error(interp("Expected numeric port value > 1024 and < 65536"
					" after %1{arg} -- not %2{value}.", arg, value));
		}
	}
}

Cmdline::Cmdline(int argc, char const ** argv) : ::Cmdline(argc, argv) {
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

} // end namespace nitro
