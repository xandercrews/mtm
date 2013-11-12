#include <stdlib.h>
#include <string.h>

#define _PROPERLY_INCLUDED
#include "domain/qsub_task.h"

namespace nitro {

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

int qsub_task::get_walltime() const {
	return 1; // todo: look up from a flag?
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
