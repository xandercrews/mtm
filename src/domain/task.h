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

	int get_exit_code() const;
	void set_exit_code(int value);

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
	int exit_code;
};

enum task_status {
	ts_ready,
	ts_active,
	ts_complete
};

/** Allow task_status to be incremented with prefix: ++status. */
task_status & operator ++(task_status &);

/** Allow task_status to be incremented with postfix: status++. */
task_status operator ++(task_status &, int);

/** Lookup friendly name for the status. */
char const * get_status_name(task_status);

} // end namespace nitro

#endif // sentry
