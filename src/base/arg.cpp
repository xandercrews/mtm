#include "base/arg.h"

arg::arg() : arg_type(atEmpty), int_value(0) {
}

arg const & make_empty() {
	static arg theEmptyArg;
	return theEmptyArg;
}

arg const & arg::Empty = make_empty();

arg::arg(int64_t value) : arg_type(atInt), int_value(value) {
}

arg::arg(int32_t value) : arg_type(atInt), int_value(value) {
}

arg::arg(int16_t value) : arg_type(atInt), int_value(value) {
}

arg::arg(int8_t value) : arg_type(atInt), int_value(value) {
}

arg::arg(uint64_t value) : arg_type(atUInt), uint_value(value) {
}

arg::arg(uint32_t value) : arg_type(atInt), int_value(value) {
}

arg::arg(uint16_t value) : arg_type(atInt), int_value(value) {
}

arg::arg(uint8_t value) : arg_type(atInt), int_value(value) {
}

arg::arg(double value) : arg_type(atDouble), double_value(value) {
}

arg::arg(char const * value) : arg_type(atCharPtr), ptr_value(value) {
}

arg::arg(std::string const & value) : arg_type(atString), str_value(&value) {
}

arg::arg(arg_compatible const * value) : arg_type(atCompatible),
		compatible_value(value) {
}

arg::arg(arg_compatible const & value) : arg_type(atCompatible),
		compatible_value(&value) {
}

std::string arg::to_string(char const * format) const {
	std::string txt;
	return append_to(txt, format);
}

std::string & arg::append_to(std::string & txt, char const * format) const {
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
