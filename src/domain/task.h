#ifndef _DOMAIN_TASK_H_
#define _DOMAIN_TASK_H_

#include <string>
#include <memory>
#include <cstdint>

namespace nitro {

class assignment;

class task {

public:
	task(char const * cmdline, assignment * asgn, uint64_t id);
	virtual ~task() {}

	assignment * get_assignment() const;
	uint64_t get_id() const;
	char const * get_cmdline() const;

	virtual int get_priority() const = 0;
	virtual int get_walltime() const = 0;
	virtual void get_validation_errors(std::string &) const = 0;
	virtual char const * get_task_style() const = 0;

	typedef std::unique_ptr<task> handle;
	static handle make(char const * cmdline, assignment * asgn, uint64_t id);

private:
	char const * cmdline;
	assignment * asgn;
	uint64_t id;

};

} // end namespace nitro

#endif // sentry
