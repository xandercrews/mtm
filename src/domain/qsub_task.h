#ifndef _DOMAIN_QSUB_TASK_H_
#define _DOMAIN_QSUB_TASK_H_

#ifndef _PROPERLY_INCLUDED
#error This header should not be directly included. Use abstract factory method instead.
#endif

#include "domain/task.h"

namespace nitro {

class qsub_task: public task {
public:
	qsub_task(char const * cmdline, char const * end_of_cmdline,
			assignment * asgn, uint64_t id);

	virtual ~qsub_task() {}
	virtual int get_priority() const;
	virtual double get_walltime_seconds() const;
	virtual void get_validation_errors(std::string &) const;
	virtual char const * get_task_style() const;
};

/**
 * Convert a walltime to raw seconds. Possible input formats include:
 *
 *     12345 -- pure seconds
 *     1:25 -- minutes:seconds
 *     4:00:05 -- hours:minutes:seconds
 *     5:12:00:00 -- days:hours:minutes:seconds
 *
 * @throws error_event(NITRO_BAD_WALLTIME_1VALUE) if value isn't formatted
 *     correctly.
 */
double walltime_to_seconds(char const * walltime);

} // end namespace nitro

#endif // sentry
