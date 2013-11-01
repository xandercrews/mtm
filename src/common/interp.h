#ifndef INTERP_H_
#define INTERP_H_
/*
 * interp.h
 *
 * Provide a better way to insert args into strings, replacing printf-style
 * mechanisms.
 *
 * Although printf-style functions are great workhorses, they have important
 * weaknesses:
 *
 *   - Some of them are not threadsafe.
 *   - Varargs can be (and often are) misused in ways that cause seg faults.
 *   - They don't provide any help in rendering objects.
 *   - Arg references cannot be reordered or repeated, which is problematic in
 *     localization.
 *   - It's easy to forget which semantics attach to a particular arg ("Was
 *     this '%s' the filename or the folder?")
 *
 * The interp() function family solves these problems while remaining fast and
 * easy to use.
 *
 * This module also provides macros to make other functions (e.g., logging,
 * exception ctors) interpolation-compatible -- sort of like the varargs
 * mechanism in C.
 *
 *  Created on: Oct 17, 2013
 *      Author: dhardman
 */

#include <string>
#include <cstdint>

#include "common/arg.h"

// First, make it possible to declare interp-style functions easily, without
// using C-style varargs in the decl.

#define arg_(n) Arg const & arg##n
#define oarg_(n) arg_(n) = Arg::Empty

/**
 * Use these macros to de-clutter declarations or implementations of interp-
 * style functions.
 */
#define ONE_ARG arg_(1)
#define TWO_ARGS ONE_ARG, arg_(2)
#define THREE_ARGS TWO_ARGS, arg_(3)
#define FOUR_ARGS THREE_ARGS, arg_(4)
#define MANY_ARGS FOUR_ARGS, oarg_(5), oarg_(6), oarg_(7), oarg_(8), oarg_(9)
#define MANY_ARGS_IMPL FOUR_ARGS, arg_(5), arg_(6), arg_(7), arg_(8), arg_(9)
#define MANY_ARGS_LIST arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9
#define MANY_OARGS oarg_(1), oarg_(2), oarg_(3), oarg_(4), oarg_(5), oarg_(6),\
	oarg_(7), oarg_(8), oarg_(9)

/**
 * To declare an interp-compatible function, just use this macro. For example:
 *
 *   - standalone function, like "void log(int level, char const * fmt, ...)":
 *
 *       DECLARE_INTERP_FUNC(void, log_event,, int level,);
 *
 *   - constructor, like "Foo(char const * fmt, ...)":
 *
 *       DELCARE_INTERP_FUNC(, Foo,);
 *
 *   - method, like "bool emit(EventID id, char const * fmt, ...) const":
 *
 *       DECLARE_INTERP_FUNC(bool, emit, const, EventID id,);
 */
#define DECLARE_INTERP_FUNC(returned, func, const_suffix, ...) \
	returned func(__VA_ARGS__ char const * fmt) const_suffix; \
	returned func(__VA_ARGS__ char const * fmt, ONE_ARG) const_suffix; \
	returned func(__VA_ARGS__ char const * fmt, TWO_ARGS) const_suffix; \
	returned func(__VA_ARGS__ char const * fmt, THREE_ARGS) const_suffix; \
	returned func(__VA_ARGS__ char const * fmt, MANY_ARGS) const_suffix

/**
 * Generate a string that has args substituted.
 *
 * @param format
 *     A template for the string we want to produce. Args are referenced by
 *     their 1-based index, as %1, %2, %3, and so forth. Format specifiers and
 *     names for args can be added with a {...} suffix. For example, if your
 *     second arg is a floating-point angle between 0 and 90 degrees, you could
 *     refer to it as "@2{angle:%.1f}", which would render it with a single
 *     digit after the decimal point, and tell any coder using it that the
 *     second arg is angle.
 * @param arg1 (arg2..arg9)
 *     Any string, number, or arg-compatible object.
 */
DECLARE_INTERP_FUNC(std::string, interp,);

/**
 * In case someone wants interp without args, for consistency.
 */
inline std::string interp(char const * format) { return format ? format : ""; }

/**
 * Helper for implementers of interp-style functions. Not normally called by
 * end users.
 */
std::string & interp_into(std::string & txt, char const * format,
		Arg const * args[], size_t max_argref);

// These lines can be uncommented to test what the macros emit. Write a simple
// .cpp file that just #includes this .h, then compile it with "gcc -E myfile".
//DECLARE_INTERP_FUNC(, Foo,);
//DECLARE_INTERP_FUNC(bool, emit, const, EventID id,);

#endif /* INTERP_H_ */
