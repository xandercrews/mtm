#ifndef _DOMAIN_CMDLINE_H_
#define _DOMAIN_CMDLINE_H_

#include "base/cmdline-base.h"

namespace nitro {

const int DEFAULT_PORT = 47000;

/**
 * Parses nitro cmdline and provides logic to react.
 */
class Cmdline : public CmdlineBase {
public:
	Cmdline(int argc, char const ** argv);
	virtual ~Cmdline();

	virtual std::string get_help() const;

protected:
	virtual char const * get_default_program_name() const;
	virtual void parse(int argc, char const ** argv);
	virtual char const * get_valid_flags() const;
	virtual char const * get_valid_options() const;
};

} // end namespace nitro

#endif // sentry
