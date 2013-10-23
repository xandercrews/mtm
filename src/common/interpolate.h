/*
 * Interpolate.h
 *
 * Provide a better way to insert args into strings, replacing printf-style mechanisms.
 *
 * Although printf-style functions are great workhorses, they have important weaknesses:
 *
 *   - Some of them are not threadsafe.
 *   - Varargs can be (and often are) misused in ways that cause seg faults.
 *   - They don't provide any help in rendering objects.
 *   - Arg references cannot be reordered or repeated, which is problematic in
 *     localization.
 *   - It's easy to forget which semantics attach to a particular arg ("Was this '%s'
 *     the filename or the folder?")
 *
 * The interpolate() function solves these problems while remaining fast and easy to use.
 *
 *  Created on: Oct 17, 2013
 *      Author: dhardman
 */

#ifndef INTERPOLATE_H_
#define INTERPOLATE_H_

#include <string>
#include <cstdint>

class Arg;

// The general form of interpolate takes many args. However, in most cases we'll only be passing
// a small number. The following overrides are optimizations so we don't always have to pass lots
// of useless pointers with every call to interpolate(). I put them first in the file so you can
// get the big picture before all the messy details.

/**
 * Generate a string that has 1 arg substituted.
 *
 * @param format A template for the string we want to produce. Args are referenced by
 *               their 1-based index, as @1, @2, @3, and so forth. Format specifiers
 *               and names for args can be added with a {...} suffix. For example, if
 *               your second arg is a floating-point angle between 0 and 90 degrees,
 *               you could refer to it as "@2{angle:%.1f}", which would render it with
 *               a single digit after the decimal point, and tell any coder using it
 *               that the second arg is angle.
 * @param arg1 A string, const char *, whole number, double, or interpolate-compatible object.
 */
std::string interpolate(char const * format, Arg const & arg1);

/**
 * Generate a string that has 2 args substituted.
 *
 * @param format A template for the string we want to produce.
 * @param arg1 A string, const char *, whole number, double, or interpolate-compatible object.
 * @param arg2 A string, const char *, whole number, double, or interpolate-compatible object.
 */
std::string interpolate(char const * format, Arg const & arg1, Arg const & arg2);

/**
 * Generate a string that has 3 args substituted.
 *
 * @param format A template for the string we want to produce.
 * @param arg1 A string, const char *, whole number, double, or interpolate-compatible object.
 * @param arg2 A string, const char *, whole number, double, or interpolate-compatible object.
 * @param arg3 A string, const char *, whole number, double, or interpolate-compatible object.
 */
std::string interpolate(char const * format, Arg const & arg1, Arg const & arg2, Arg const & arg3);



/**
 * Any classes that we write in the future that we want to be able to
 * interpolate can derive from this interface and automatically become
 * usable with interpolate() -- without modifying any code in this
 * module.
 */
class ArgCompatible {
public:
	virtual ~ArgCompatible();
	virtual std::string to_string(char const * format = 0) const;
};

/**
 * A class that wraps any argument to interpolate(). This class is intended
 * to have a lifetime equal to the interpolate() call where it is created,
 * and is exceedingly lightweight. Essentially, it stores a pointer or int
 * in an internal union, and then converts it to text as needed.
 */
class Arg {

	// Set up a tagged union to hold the different stuff we might
	// want to interpolate.
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
	friend Arg const & make_empty(); // the only func that we'll allow to call default ctor

	// Copy ctor and assignment operator private because these objects
	// should only be used with a lifetime equal to the interpolate()
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
	Arg(ArgCompatible const * value);

	// Mainly used for testing.
	std::string to_string(char const * format = NULL) const;

	// Called by interpolate() to add to string we're building.
	std::string & append(std::string & txt, char const * format = NULL) const;
	
	// The placeholder arg.
	static Arg const & Empty;
};

/**
 * Generate a string that has up to 9 args substituted.
 *
 * @param format A template for the string we want to produce.
 */
std::string interpolate(char const * txt, Arg const & arg1, Arg const & arg2,
		Arg const & arg3, Arg const & arg4, Arg const & arg5=Arg::Empty,
		Arg const & arg6=Arg::Empty, Arg const & arg7=Arg::Empty,
		Arg const & arg8=Arg::Empty, Arg const & arg9=Arg::Empty);


#endif /* INTERPOLATE_H_ */
