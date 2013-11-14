#ifndef _DOMAIN_ASSIGNMENT_H_
#define _DOMAIN_ASSIGNMENT_H_

#include <list>
#include <mutex>
#include <string>

#include "domain/task.h"

namespace nitro {

/**
 * The unit of delegated work -- typically a few dozen or a few hundred
 * individual commands.
 *
 * Assignments are thread-safe; you can interact with them from multiple
 * threads in parallel.
 */
class assignment {
public:
	/**
	 * Used by coord_engine; caller will subsequently call add_task() until
	 * assignment is populated.
	 */
	assignment(char const * id);

	/**
	 * Used by worker engine; caller will pass data from a json message in
	 * @param lines to fully populate assignment.
	 */
	assignment(char const * id, char const * lines);

	char const * get_id() const;

	/**
	 * Return true if both ready list and active list are empty. Can become
	 * incomplete after being complete, if new tasks are added.
	 */
	bool is_complete() const;

	/**
	 * Count how many tasks are in each state. NULL can be passed for any
	 * parameters that are uninteresting.
	 *
	 * @return total count
	 */
	unsigned get_counts(unsigned * complete=nullptr,
			unsigned * active = nullptr, unsigned * ready=nullptr);

	/**
	 * Return a json string suitable for transmitting to a worker to request
	 * that an assignment be accepted.
	 */
	std::string get_request_msg() const;

	/**
	 * Return a json string suitable for reporting how far along we are
	 * in processing an assignment.
	 */
	std::string get_status_msg() const;

	/**
	 * Add a task that's ready for execution.
	 */
	task * ready_task(task::id_type tid, char const * cmdline,
			char const * end_of_cmdline=nullptr);

	/**
	 * Mark a task active and return its cmdline so we can launch it.
	 */
	char const * activate_task(task::id_type tid);

	/**
	 * Mark a task as complete; return true if entire assignment is complete.
	 */
	bool complete_task(task::id_type tid);

	typedef std::list<task::handle> tasklist_t;
	tasklist_t const & get_list_by_status(task_status status) const;

	typedef std::unique_ptr<assignment> handle;

private:
	void fill_from_lines(char const * lines);

	task * _ready_task(task::id_type tid, char const * cmdline,
			char const * end_of_cmdline);

	bool _is_complete() const;

	std::string id;
	tasklist_t lists[3];
	mutable std::mutex mutex;
};

} // end namespace nitro

#endif // sentry
