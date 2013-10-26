/*
 * interp_arg.h
 *
 * Define the wrapper that remembers args to interp-style functions.
 */

#ifndef INTERP_ARG_H_
#define INTERP_ARG_H_

#include <string>

/**
 * Any classes that we write in the future that we want to be able to
 * interp can derive from this interface and automatically become
 * usable with interp() -- without modifying any code in this
 * module.
 */
class ArgCompatible {
public:
	virtual ~ArgCompatible();
	virtual std::string to_string(char const * format = 0) const;
};

/**
 * A class that wraps any argument to interp(). This class is intended
 * to have a lifetime equal to the interp() call where it is created,
 * and is exceedingly lightweight. Essentially, it stores a pointer or int
 * in an internal union, and then converts it to text as needed.
 */
class Arg {

	// Set up a tagged union to hold the different stuff we might
	// want to interp.
	enum Type {
		atEmpty,
		atInt,
		atUInt,
		atDouble,
		atCharPtr,
		atString,
		atCompatible,
	} arg_type;

	union {
		int64_t int_value;
		uint64_t uint_value;
		double double_value;
		char const * ptr_value;
		std::string const * str_value;
		ArgCompatible const * compatible_value;
	};

	// Default ctor used to create the static Empty arg is private so
	// it can't be created except where we want it.
	Arg();
	friend Arg const & make_empty(); // the only func that calls default ctor

	// Copy ctor and assignment operator private because these objects
	// should only be used with a lifetime equal to the interp()
	// call where they are created.
	Arg(Arg const &);
	Arg & operator =(Arg const &);

public:
	Arg(int64_t value);
	Arg(int32_t value);
	Arg(int16_t value);
	Arg(int8_t value);
	Arg(uint64_t value);
	Arg(uint32_t value);
	Arg(uint16_t value);
	Arg(uint8_t value);
	Arg(double value);
	Arg(char const * value);
	Arg(std::string const & value);
	Arg(ArgCompatible const & value);
	Arg(ArgCompatible const * value);

	// Mainly used for testing.
	std::string to_string(char const * format = NULL) const;

	// Called by interp() to add to string we're building.
	std::string & append_to(std::string & txt, char const * format = NULL) const;
	
	// The placeholder arg.
	static Arg const & Empty;
};

#endif /* INTERP_ARG_H_ */
