#include <string.h>

#include "common/interpolate.h"

Arg::Arg() : arg_type(atEmpty), int_value(0) {
}

Arg const & make_empty() {
	static Arg theEmptyArg;
	return theEmptyArg;
}

Arg const & Arg::Empty = make_empty();

Arg::Arg(int64_t value) : arg_type(atInt), int_value(value) {
}

Arg::Arg(int32_t value) : arg_type(atInt), int_value(value) {
}

Arg::Arg(int16_t value) : arg_type(atInt), int_value(value) {
}

Arg::Arg(int8_t value) : arg_type(atInt), int_value(value) {
}

Arg::Arg(uint64_t value) : arg_type(atUInt), uint_value(value) {
}

Arg::Arg(uint32_t value) : arg_type(atInt), int_value(value) {
}

Arg::Arg(uint16_t value) : arg_type(atInt), int_value(value) {
}

Arg::Arg(uint8_t value) : arg_type(atInt), int_value(value) {
}

Arg::Arg(double value) : arg_type(atInt), double_value(value) {
}

Arg::Arg(char const * value) : arg_type(atCharPtr), ptr_value(value) {
}

Arg::Arg(std::string const & value) : arg_type(atString), str_value(&value) {
}

Arg::Arg(ArgCompatible const * value) : arg_type(atCompatible), compatible_value(value) {
}

std::string Arg::to_string(char const * format) const {
	std::string txt;
	return append(txt);
}

std::string & Arg::append(std::string & txt, char const * format) const {
	switch (arg_type) {
	case atUInt:
		{
			char buf[25];
			if (!format || !*format) {
				format = "%u";
			}
			sprintf(buf, format, uint_value);
			txt += buf;
		}
		break;
	case atInt:
		{
			char buf[25];
			if (!format || !*format) {
				format = "%d";
			}
			sprintf(buf, format, int_value);
			txt += buf;
		}
		break;
	case atDouble:
		{
			char buf[25];
			if (!format || !*format) {
				format = "%f";
			}
			sprintf(buf, format, double_value);
			txt += buf;
		}
		break;
	case atCharPtr:
		if (ptr_value == NULL) {
			txt += "(null)";
		} else {
			txt += ptr_value;
		}
		break;
	case atString:
		if (!str_value->empty()) {
			txt += *str_value;
		}
		break;
	case atCompatible:
		if (compatible_value) {
			txt += compatible_value->to_string(format);
		} else {
			txt += "(null)";
		}
		break;
	default:
		break;
	}
	return txt;
}

const size_t FMTSPEC_BUF_LEN = 64;

/**
 * Given an arg ref such as %1 or %2{file name} or %3{angle of refraction:%3.2f},
 * identify the format specifier (which is NULL for the first two examples, and
 * "%3.2f" for the last example).
 *
 * @param first_char_after
 * 		Points to the char that will contain a curly brace if the argref is fancy.
 * @param fmtspec_buf
 * 		A buf of size FMTSPEC_BUF_LEN into which we can copy the format specifier,
 * 		if we find one.
 * @param last_char_of_argref (out)
 * 		On exit, points to final char of this argref, whether it be a numeral or
 * 		a closing curly brace.
 *
 * @return A format specifier (stored in fmtspec_buf) or NULL if none was found.
 */
static char const * get_fmt_spec(char const * first_char_after, char * fmtspec_buf,
		char const * & last_char_of_argref) {

	// By default, assume that we won't find a format specifier.
	char const * fmtspec = NULL;

	// By default, assume that the last char of the argref is the char before
	// the one we're going to look at.
	last_char_of_argref = first_char_after - 1;

	if (*first_char_after == '{') {
		auto p = first_char_after + 1;

		// Check for doubled open curly. If present, curly is literal.
		if (*p == '{') {
			last_char_of_argref = first_char_after;
		} else {

			// Do we have a terminator?
			auto end_of_fmtspec = strchr(p, '}');

			// Is it plausible?
			if (end_of_fmtspec < first_char_after + FMTSPEC_BUF_LEN) {

				// If we don't see a %s, %d, %f, or similar right away...
				if (*p != '%' || end_of_fmtspec == p + 1) {

					// Look for specs in the form {label:%3.2f}.
					auto colon = strchr(p, ':');
					if (colon && colon < end_of_fmtspec - 2 && colon[1] == '%') {
						fmtspec = colon + 1;
					}
				} else {
					fmtspec = p;
				}

				// If we get here, we probably found a format spec, unless it was
				// malformed in some way. We will be optimistic. Worst case is that
				// a garbled input string produces a garbled output string.
				if (fmtspec) {

					// Can we fit it in the buffer we've been given?
					auto fmtspec_len = end_of_fmtspec - fmtspec;
					if (fmtspec_len < sizeof(fmtspec_buf) - 1) {
						strncpy(fmtspec_buf, fmtspec, fmtspec_len);
						fmtspec_buf[fmtspec_len] = 0;
						fmtspec = fmtspec_buf;
					}
				}
				last_char_of_argref = end_of_fmtspec;
			}
		}
	}
	return fmtspec;
}

/**
 * Fill a string with a template plus interpolated args.
 */
static void expand(char const * format, std::string & txt, Arg const * args[], size_t max_idx) {
	char const * end = strchr(format, 0);

	// Reserve memory all at once, rather than piecemeal. This makes all the += stuff
	// later in this function cheap; in a release build, it simply converts into an inlined
	// set of a char in a buffer.
	txt.reserve((end - format) * 2);

	auto max_idx_char = '0' + max_idx;
	for (char const * p = format; p < end; ++p) {
		char c = *p;
		// Look for potential arg refs.
		if (c == '%' && p < end - 1) {
			char c2 = p[1];

			// Doubled %% gets expanded to single.
			if (c2 == '%') {
				txt += c2;
				++p;

			// Valid arg ref, in correct range. Expand it.
			} else if (c2 >= '1' && c2 <= max_idx_char) {

				Arg const * arg = args[c2 - '1'];
				// Reserve a place to copy the format specifier, if one exists.
				char fmtspec_buf[FMTSPEC_BUF_LEN];
				// Go look for it. This call also adjusts p as needed.
				char const * fmtspec = get_fmt_spec(p + 2, fmtspec_buf, p);
				arg->append(txt, fmtspec);

			} else {
				txt += c;
			}
		} else {
			txt += c;
		}
	}
}

std::string interpolate(char const * format, Arg const & arg1, Arg const & arg2,
		Arg const & arg3, Arg const & arg4, Arg const & arg5, Arg const & arg6,
		Arg const & arg7, Arg const & arg8, Arg const & arg9) {
	std::string txt;
	Arg const * args[] = {&arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9};
	expand(format, txt, args, 9);
	return txt; // optimized w/ move semantics in C++ 11
}

std::string interpolate(char const * format, Arg const & arg1, Arg const & arg2,
		Arg const & arg3) {
	std::string txt;
	Arg const * args[] = {&arg1, &arg2, &arg3};
	expand(format, txt, args, 3);
	return txt; // optimized w/ move semantics in C++ 11
}

std::string interpolate(char const * format, Arg const & arg1, Arg const & arg2) {
	std::string txt;
	Arg const * args[] = {&arg1, &arg2};
	expand(format, txt, args, 2);
	return txt; // optimized w/ move semantics in C++ 11
}

std::string interpolate(char const * format, Arg const & arg1) {
	std::string txt;
	Arg const * args[] = {&arg1};
	expand(format, txt, args, 1);
	return txt; // optimized w/ move semantics in C++ 11
}
