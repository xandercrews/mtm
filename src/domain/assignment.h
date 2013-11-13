#ifndef _DOMAIN_ASSIGNMENT_H_
#define _DOMAIN_ASSIGNMENT_H_

#include <map>
#include <string>

#include "domain/task.h"

namespace nitro {

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
	 * Return a json string suitable for transmitting to a worker to request
	 * that an assignment be accepted.
	 */
	std::string get_request_msg() const;

	/**
	 * Return a json string suitable for reporting how far along we are
	 * in processing an assignment.
	 */
	std::string get_status_msg() const;

	task * add_task(task::id_type tid, char const * cmdline,
			char const * end_of_cmdline=nullptr);

	void complete_task(task::id_type tid);

	typedef std::map<task::id_type, task::handle> taskmap_t;
	taskmap_t const & get_taskmap() const;

private:
	void fill_from_lines(char const * lines);

	std::string id;
	taskmap_t taskmap;
};

} // end namespace nitro

#endif // sentry
