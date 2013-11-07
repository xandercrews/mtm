#ifndef _DOMAIN_LEADER_ENGINE_H_
#define _DOMAIN_LEADER_ENGINE_H_

#include <condition_variable>
#include <mutex>
#include <vector>

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

	virtual int run();

	typedef std::vector<std::string> hostlist_t;
	hostlist_t const & get_hostlist() const;

	/**
	 * Call from any thread that needs to wait until engine has bound its ports
	 * and is ready to communicate.
	 */
	void wait_until_ready() const;

	/**
	 * An engine handle manages the lifetime of an engine instance. When the
	 * handle goes out of scope, the engine automatically shuts down (cleanly),
	 * and all resources are released. It goes dark on the network.
	 *
	 * handle objects can be passed from one owner to another, like batons.
	 */
	typedef std::unique_ptr<coord_engine> handle;

private:
	hostlist_t hostlist;
	mutable bool ready;
	mutable std::mutex ready_mutex;
	mutable std::condition_variable ready_signal;
};

} // end namespace nitro

#endif // sentry
