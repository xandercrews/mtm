#include <string.h>

#include "domain/task.h"
#define _PROPERLY_INCLUDED
#include "domain/qsub_task.h"

namespace nitro {

task::task(char const * cmdline, assignment * asgn, uint64_t id) :
	cmdline(cmdline), asgn(asgn), id(id) {
}

assignment * task::get_assignment() const {
	return asgn;
}

uint64_t task::get_id() const {
	return id;
}

char const * task::get_cmdline() const {
	return cmdline;
}

task::handle task::make(char const * cmdline, assignment * asgn, uint64_t id) {
	if (cmdline) {
		while (isspace(*cmdline)) {
			++cmdline;
		}
		if (*cmdline) {
			if (strncmp(cmdline, "qsub", 4) == 0) {
				return handle(new qsub_task(cmdline, asgn, id));
			}
		}
	}
	return handle(nullptr);
}

} // end namespace nitro
