#define _PROPERLY_INCLUDED
#include "domain/qsub_task.h"

namespace nitro {

int qsub_task::get_priority() const {
	return 1; // todo; return value of -p flag
}

int qsub_task::get_walltime() const {
	return 100; // todo: look up from -a flag
}

void qsub_task::get_validation_errors(std::string &) const {
	//todo: complain about invalid flags
}

char const * qsub_task::get_task_style() const {
	return "qsub";
}

}
