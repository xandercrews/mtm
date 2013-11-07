#ifndef _BASE_STRUTIL_H_
#define _BASE_STRUTIL_H_

#include <string>
#include <vector>

/// trim from start
std::string & ltrim(std::string &s);

/// trim from end
std::string & rtrim(std::string &s);

/// trim from both ends
std::string & trim(std::string &s);

/// Split a string at an arbitrary delimiter, and push any items that are
/// found into a vector of strings.
std::vector<std::string> & split(std::string const & s, char delim,
		std::vector<std::string> & elems);

/// Split a string at an arbitrary delimiter, and push any items that are
/// found into a vector of strings. Return the newly built vector.
std::vector<std::string> split(std::string const & s, char delim);

#endif // sentry
