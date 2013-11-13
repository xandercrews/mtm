#ifndef _DOMAIN_COORD_ENGINE_H_
#define _DOMAIN_COORD_ENGINE_H_

#include "domain/engine.h"

namespace nitro {

/**
 * The engine used when the app is in "coordinator" mode, giving instructions to
 * workers.
 */
class coord_engine : public engine {
public:
	coord_engine(cmdline const &);
	virtual ~coord_engine();

	virtual int do_run();

	typedef std::vector<std::string> stringlist_t;
	typedef std::unique_ptr<stringlist_t> assignment_t;

	stringlist_t const & get_hostlist() const;

	assignment_t next_assignment();
	void prioritize(assignment_t & asgn);
	void distribute(assignment_t & asgn);
	void enroll_workers(int eid);
	void enroll_workers_multi(int eid);

private:
	struct data_t;
	data_t * data;

	void init_hosts(cmdline const &);
	void progress_reporter();
	bool report_progress;
	int reporter_port;
};

} // end namespace nitro

#endif // sentry
