#include <string.h>

#include "domain/task.h"
#define _PROPERLY_INCLUDED
#include "domain/qsub_task.h"

namespace nitro {

task::task(char const * cmdline, char const * end_of_cmdline,
		assignment * asgn, task::id_type id) :
	cmdline(cmdline, end_of_cmdline ? end_of_cmdline : strchr(cmdline, 0)),
	asgn(asgn), id(id), exit_code(0) {
}

int task::get_exit_code() const {
	return exit_code;
}

void task::set_exit_code(int value) {
	exit_code = value;
}

char const * get_status_name(task_status x) {
	switch (x) {
	case task_status::ts_ready: return "ready";
	case task_status::ts_active: return "active";
	case task_status::ts_complete: return "complete";
	default: return "unknown";
	}
}

task_status & operator ++(task_status & x) {
	int next = static_cast<int>(x) + 1;
	x = static_cast<task_status>(next);
	return x;
}

task_status operator ++(task_status & x, int) {
	// Implement postfix in terms of prefix.
	task_status before_being_incremented = x;
	operator++(x);
	return before_being_incremented;
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
