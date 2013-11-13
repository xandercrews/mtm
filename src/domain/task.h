#ifndef _DOMAIN_TASK_H_
#define _DOMAIN_TASK_H_

#include <string>
#include <memory>
#include <cstdint>

namespace nitro {

class assignment;

class task {

public:
	typedef uint64_t id_type;

	virtual ~task() {}

	assignment * get_assignment() const;

	id_type get_id() const;

	char const * get_cmdline() const;
	virtual int get_priority() const = 0;
	virtual double get_walltime_seconds() const = 0;
	virtual void get_validation_errors(std::string &) const = 0;
	virtual char const * get_task_style() const = 0;

	typedef std::unique_ptr<task> handle;

	/**
	 * Abstract factory method that generates appropriate derived task type,
	 * based on null-terminated cmdline.
	 */
	static handle make(char const * cmdline, assignment * asgn, id_type id);

	/**
	 * Abstract factory method that generates appropriate derived task type,
	 * based on a subset of a larger string.
	 */
	static handle make(char const * cmdline, char const * end_of_cmdline,
			assignment * asgn, id_type id);

protected:
	task(char const * cmdline, char const * end_of_cmdline, assignment * asgn,
			id_type id);

private:
	std::string cmdline;
	assignment * asgn;
	uint64_t id;

};

} // end namespace nitro

#endif // sentry
