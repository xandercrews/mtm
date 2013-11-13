#include <stdlib.h>
#include <string.h>

#include "base/error.h"

#include "domain/event_codes.h"
#define _PROPERLY_INCLUDED
#include "domain/qsub_task.h"

using namespace nitro::event_codes;

namespace nitro {

qsub_task::qsub_task(char const * cmdline, char const * end_of_cmdline,
		assignment * asgn, uint64_t id) :
	task(cmdline, end_of_cmdline, asgn, id) {
}

int qsub_task::get_priority() const {
	char const * p = strstr(get_cmdline(), " -p");
	if (p && p[2] == ' ') {
		char * end;
		auto val = strtol(p + 3, &end, 10);
		if (val >= -1024 && val <= 1023) {
			return val;
		}
	}
	return 0;
}

double walltime_to_seconds(char const * walltime) {
	double value = 0.0;
	char const * end = strchr(walltime, ' ');
	if (!end) {
		end = strchr(walltime, 0);
	}
	char buf[128];
	if (static_cast<size_t>(end - walltime) < sizeof(buf) - 1) {
		strncpy(buf, walltime, end - walltime);
		buf[end - walltime] = 0;
	}
	int multiplier = 1;
	while (true) {
		auto p = strchr(buf, 0);
		while (p > buf && *p != ':') {
			--p;
		}
		if (*p == ':') {
			++p;
		}
		char * end_of_segment;
		auto val = strtol(p, &end_of_segment, 10);
		if (*end_of_segment != 0) {
			throw ERROR_EVENT(NITRO_BAD_WALLTIME_1VALUE,
					std::string(walltime, end));
		}
		val *= multiplier;
		value += val;
		if (multiplier < 3600) {
			multiplier *= 60;
		} else {
			multiplier = 86400;
		}
		if (p > buf) {
			p[-1] = 0;
		} else {
			break;
		}
	}
	return value;
}

double qsub_task::get_walltime_seconds() const {
	// TODO: Look for -l walltime=value
	return 1;
}

void qsub_task::get_validation_errors(std::string & errors) const {
	auto x = get_cmdline();
	char const * const BAD_FLAGS = "tfhIx";
	auto dashF = strstr(x, " -F");
	while (x) {
		auto flag = strstr(x, " -");
		if (!flag) {
			break;
		}
		// TODO: get more robust; this is just quick and dirty
		if (flag == dashF) {
			if (flag[3] == ' ' && flag[4] == '"') {
				x = strchr(flag + 5, '"');
			}
		} else if (strchr(BAD_FLAGS, flag[2])) {
			errors += "The -";
			errors += flag[2];
			errors += " flag is not supported. ";
		} else if (flag[2] == 'W') {
			if (strncmp(flag + 4, "job_radix", 9) == 0) {
				errors += "-W job_radix is not supported. ";
			} else if (strncmp(flag + 4, "interactive=true", 16) == 0) {
				errors += "-W interactive=true is not supported. ";
			}
		}
	}
}

char const * qsub_task::get_task_style() const {
	return "qsub";
}

}
