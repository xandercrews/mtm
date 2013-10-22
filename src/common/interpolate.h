/*
 * Interpolate.h
 *
 *  Created on: Oct 17, 2013
 *      Author: dhardman
 */

#ifndef INTERPOLATE_H_
#define INTERPOLATE_H_

#include <string>

class Arg {
	enum ArgType {
		atEmpty,
		atInt,
		atDouble,
		atCharPtr,
		atString
	};
	ArgType arg_type;
	union {
		int int_value;
		double double_value;
		char const * ptr_value;
		std::string const * str_value;
	};
	Arg();
public:
	Arg(int value);
	Arg(double value);
	Arg(char const * value);
	Arg(std::string const & value);
	
	char const * to_string() const;
	
	static Arg const & Empty;
};

std::string interpolate(char const * txt, Arg const & arg1);
std::string interpolate(char const * txt, Arg const & arg1, Arg const & arg2);
std::string interpolate(char const * txt, Arg const & arg1, Arg const & arg2, Arg const & arg3);
std::string interpolate(char const * txt, Arg const & arg1, Arg const & arg2, Arg const & arg3, Arg const & arg4, Arg const & arg5=Arg::Empty);

#endif /* INTERPOLATE_H_ */
