#ifndef _DOMAIN_CMDLINE_H_
#define _DOMAIN_CMDLINE_H_

#include "base/cmdline-base.h"

namespace nitro {

const int DEFAULT_REPLY_PORT = 47000;
const int DEFAULT_PUBLISH_PORT = 47001;
const char * const DEFAULT_MULTICAST_INTERFACE = "eth0";
const int DEFAULT_REPORTER_PORT = 35000;

/**
 * Parses nitro cmdline and provides logic to react.
 */
class cmdline : public cmdline_base {
public:
	cmdline(int argc, char const ** argv);
	virtual ~cmdline();

	virtual std::string get_help() const;

protected:
	virtual char const * get_default_program_name() const;
	virtual void parse(int argc, char const ** argv);
	virtual char const * get_valid_flags() const;
	virtual char const * get_valid_options() const;

private:
	int validate_port(char const * port_switch, int exclusive_port = 0);
};

} // end namespace nitro

#endif // sentry
