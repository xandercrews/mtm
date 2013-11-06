/*
 * cmdline.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#include <string.h>

#include "base/interp.h"
#include "domain/cmdline.h"

namespace nitro {

char const * cmdline::get_valid_flags() const {
	return "--help|-h|--follow|-f";
}

char const * cmdline::get_valid_options() const {
	return "--listenport|-l|--talkport|-t";
}

char const * cmdline::get_default_program_name() const {
	return "nitro";
}

int cmdline::validate_port(char const * port_switch, int exclusive_port) {
	auto n = 0;
	char const * port = get_option(port_switch);
	if (port) {
		auto end = strchr(port, 0);
		char * last_digit;
		n = strtol(port, &last_digit, 10);
		if (n < 1024 || n > 65536 || last_digit != end) {
			add_error(interp("Expected numeric port value > 1024 and < 65536"
					" after %1{port_switch}, not \"%2{value}\".",
					port_switch, n));
		} else if (n == exclusive_port) {
			add_error(interp("Expected %1{port_switch} to get a unique value,"
					" but port %2{value} is re-used.", port_switch, n));
		}
	}
	return n;
}

void cmdline::parse(int argc, char const ** argv) {
	cmdline_base::parse(argc, argv);
	auto n = validate_port("--listenport");
	validate_port("--talkport", n);
}

cmdline::cmdline(int argc, char const ** argv) {
	parse(argc, argv);
}

cmdline::~cmdline() {
}


std::string cmdline::get_help() const {
	return interp(
		"%1{errors}\n%2{progname} -- run batches of similar jobs at high speed\n"
		"\n"
		"  Syntax: %2{progname} [flags] [options] [batch file(s)]\n"
		"\n"
		"    Flags:\n"
		"      --follow or -f     -- Take work from another %2{progname} process.\n"
		"      --help or -h       -- Display this screen.\n"
		"\n"
		"    Options:\n"
		"      --listenport or -l -- Listen on specific port (%3{deflport} is default).\n"
		"      --talkport or -t   -- Talk on specific port (%4{deftport} is default).\n"
		"\n",
		get_errors(), get_program_name(), DEFAULT_LISTEN_PORT, DEFAULT_TALK_PORT
		);
}

} // end namespace nitro
