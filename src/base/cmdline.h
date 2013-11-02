#ifndef _BASE_CMDLINE_H_
#define _BASE_CMDLINE_H_

#include <string>
#include <vector>
#include <utility>

/**
 * Defines an interface for parsing a cmdline and providing logic to react.
 */
class Cmdline {
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

	bool is_flag(char const * arg) const;
	bool is_option(char const * arg) const;
	virtual const char * get_valid_flags() = 0;
	virtual const char * get_valid_options() = 0;

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
	virtual std::string get_help() const = 0;

private:
	char const * program_name;
	Cmdline::Options options;
	Cmdline::Strings positional_args;
	Cmdline::Strings flags;
	std::string errors;

protected:
	virtual char const * get_default_program_name() const = 0;
	virtual void parse(int argc, char const ** argv);
	void add_error(std::string const & txt);
};

/**
 * Used by implementers of a cmdline. Scans a string that lists switches, in
 * the form "--switch|--s|--flag|--f", and tells whether the item appears in
 * the list.
 */
bool in_alternatives_str(char const * item, char const * alternatives);

/**
 * Used by implementers of a derived class of Cmdline. Compares a possibly
 * short form or long form to a known long form of a switch, and returns true
 * if there's a match.
 *
 *     "-h" and "--help" are both matches for the long form "--help"
 */
bool matches_switch(char const * long_or_short_form, char const * long_form);

#endif // sentry
