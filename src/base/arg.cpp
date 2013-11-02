#include "base/arg.h"

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

Arg::Arg(double value) : arg_type(atDouble), double_value(value) {
}

Arg::Arg(char const * value) : arg_type(atCharPtr), ptr_value(value) {
}

Arg::Arg(std::string const & value) : arg_type(atString), str_value(&value) {
}

Arg::Arg(ArgCompatible const * value) : arg_type(atCompatible),
		compatible_value(value) {
}

Arg::Arg(ArgCompatible const & value) : arg_type(atCompatible),
		compatible_value(&value) {
}

std::string Arg::to_string(char const * format) const {
	std::string txt;
	return append_to(txt);
}

std::string & Arg::append_to(std::string & txt, char const * format) const {
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
