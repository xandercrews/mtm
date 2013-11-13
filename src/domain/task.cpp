#include <string.h>

#include "domain/task.h"
#define _PROPERLY_INCLUDED
#include "domain/qsub_task.h"

namespace nitro {

task::task(char const * cmdline, char const * end_of_cmdline,
		assignment * asgn, task::id_type id) :
	cmdline(cmdline, end_of_cmdline ? end_of_cmdline : strchr(cmdline, 0)),
	asgn(asgn), id(id) {
}

assignment * task::get_assignment() const {
	return asgn;
}

task::id_type task::get_id() const {
	return id;
}

char const * task::get_cmdline() const {
	return cmdline.c_str();
}

task::handle task::make(char const * cmdline, assignment * asgn,
		task::id_type id) {
	return task::make(cmdline, nullptr, asgn, id);
}

task::handle task::make(char const * cmdline, char const * end_of_cmdline,
		assignment * asgn, task::id_type id) {
	if (cmdline) {
		if (!end_of_cmdline) {
			end_of_cmdline = strchr(cmdline, 0);
		}
		while (isspace(*cmdline) && cmdline < end_of_cmdline) {
			++cmdline;
		}
		if (*cmdline && cmdline < end_of_cmdline) {
			if (strncmp(cmdline, "qsub", 4) == 0) {
				return handle(new qsub_task(cmdline, end_of_cmdline, asgn, id));
			}
		}
	}
	return handle(nullptr);
}

} // end namespace nitro
