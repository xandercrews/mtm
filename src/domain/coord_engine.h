#ifndef _DOMAIN_LEADER_ENGINE_H_
#define _DOMAIN_LEADER_ENGINE_H_

#include <condition_variable>
#include <mutex>
#include <vector>

#include "base/file_lines.h"

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

	/**
	 * An engine handle manages the lifetime of an engine instance. When the
	 * handle goes out of scope, the engine automatically shuts down (cleanly),
	 * and all resources are released. It goes dark on the network.
	 *
	 * handle objects can be passed from one owner to another, like batons.
	 */
	typedef std::unique_ptr<coord_engine> handle;

private:
	std::unique_ptr<file_lines> current_batch_file;
	stringlist_t hostlist;
	stringlist_t batches;
	void * requester;
	void * _job_annonce_pgm;
	void * _job_annonce_ipc;
	stringlist_t _workers;
};

} // end namespace nitro

#endif // sentry
