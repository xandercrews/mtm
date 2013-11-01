#ifndef _NITRO_COMMON_CMDLINE_H_
#define _NITRO_COMMON_CMDLINE_H_

#include <string>
#include <vector>
#include <utility>

namespace nitro {

const int DEFAULT_PORT = 47000;

/**
 * Parses nitro cmdline and provides logic to react.
 */
class Cmdline {
	struct Data;
	Data * data;

public:
	typedef std::pair<char const *, char const *> Option;
	typedef std::vector<Option> Options;
	typedef std::vector<char const *> Strings;

	Cmdline(int argc, char const ** argv);
	virtual ~Cmdline();

	Options const & get_options() const;
	Strings const & get_positional_args() const;
	Strings const & get_flags() const;
	std::string const & get_errors() const;

	/**
	 * @return argv[0]
	 */
	char const * get_program_name() const;

	/**
	 * Look for an option with the specified name. If found, return its value.
	 * If not, return NULL.
	 *
	 * @param full_option_name
	 *     Always use the long form. Short-form equivalents are checked automatically.
	 */

	char const * get_option(char const * full_option_name) const;
	/**
	 * Return true if a flag is present.
	 *
	 * @param flag_name
	 *     Always use the long form. Short-form equivalents are checked automatically.
	 */
	bool has_flag(char const * full_flag_name) const;

	/**
	 * @return true if we should display help, either because we have errors or because
	 *     caller asked for it explicitly.
	 */
	bool help_needed() const;

	/**
	 * Return help for the program.
	 */
	std::string get_help() const;
};

} /* namespace nitro */

#endif /* CMDLINE_H_ */
