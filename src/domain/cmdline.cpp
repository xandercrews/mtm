/*
 * cmdline.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#include <string.h>
#include <stdlib.h>

#include "base/interp.h"
#include "domain/cmdline.h"
#include "domain/event_codes.h"

using namespace nitro::event_codes;

namespace nitro {

char const * cmdline::get_valid_flags() const {
	return "--help|-h|--follow|-f";
}

/**
 * TODO: this is not the way I want to do things in the long run. I'd like to
 * call a base-class method that defines an option with a replacement text and
 * a validation function. So, for example, I'd like to do something like this:
 *
 *   cmdline.add_option("--inputfile", "FILE", file_must_exist);
 *
 * For the time being, this is as far as I got.
 */
char const * cmdline::get_valid_options() const {
	return "--rrport|-r|--psport|-p|--workfor|-w|--exechost|-e"
			"|--interface|-i|--linger|-l";
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
			add_error(ERROR_EVENT(
					NITRO_EXPECTED_PORT_NUM_AFTER_1SWITCH_NOT_2VAL,
					port_switch, n));
		} else if (n == exclusive_port) {
			add_error(ERROR_EVENT(
					NITRO_EXPECTED_UNIQUE_1SWITCH_BUT_2NUM_REUSED,
					port_switch, n));
		}
	}
	return n;
}

void cmdline::parse(int argc, char const ** argv) {
	cmdline_base::parse(argc, argv);
	auto n = validate_port("--rrport");
	validate_port("--psport", n);
}

cmdline::cmdline(int argc, char const ** argv) {
	parse(argc, argv);
}

cmdline::~cmdline() {
}


std::string cmdline::get_help() const {
	std::string e;
	for (auto err: get_errors()) {
		if (!e.empty()) {
			e += "\n";
		}
		e += err.what();
	}
	return interp(
		"%1{errors}\n%2{progname} -- run batches of similar jobs at high speed\n"
		"\n"
		"  Syntax: %2{progname} [flags] [options] [batch file(s)]\n"
		"\n"
		"    Flags:\n"
		"      --help or -h       -- Display this screen.\n"
		"      --linger or -l     -- Wait for terminate message before exiting.\n"
		"\n"
		"    Options:\n"
		"      --workfor or -w    -- Take work from coordinator on specified host.\n"
		"                            (Precludes batch files on cmdline.)\n"
		"      --rrport or -r     -- Request/reply on this port (%3 is default).\n"
		"      --psport or -p     -- Publish/subscribe on this port (%4 is default).\n"
        "      --interface or -i  -- NIC for multicast messages (%5{iface} is default).\n"
		"\n",
		e, get_program_name(), DEFAULT_REQREP_PORT, DEFAULT_PUBSUB_PORT,
		DEFAULT_MULTICAST_INTERFACE
		);
}

} // end namespace nitro
