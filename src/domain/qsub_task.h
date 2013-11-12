#ifndef _DOMAIN_QSUB_TASK_H_
#define _DOMAIN_QSUB_TASK_H_

#ifndef _PROPERLY_INCLUDED
#error This header shouldn't be directly included. Use abstract factory method instead.
#endif

#include "domain/task.h"

namespace nitro {

class qsub_task: public task {
public:
	qsub_task(char const * cmdline, assignment * asgn, uint64_t id);
	virtual ~qsub_task() {}
	virtual int get_priority() const;
	virtual int get_walltime() const;
	virtual void get_validation_errors(std::string &) const;
	virtual char const * get_task_style() const;
};

} // end namespace nitro

#endif // sentry
