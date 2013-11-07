#ifndef _DOMAIN_WORKER_ENGINE_H_
#define _DOMAIN_WORKER_ENGINE_H_

#include "domain/engine.h"

namespace nitro {

/**
 * The engine used when the app is in "worker" mode, waiting for instructions
 * from a coordinator.
 */
class worker_engine : public engine {

public:
	worker_engine(cmdline const & cmdline);
	virtual ~worker_engine();

	/**
	 * An engine handle manages the lifetime of an engine instance. When the
	 * handle goes out of scope, the engine automatically shuts down (cleanly),
	 * and all resources are released. It goes dark on the network.
	 *
	 * handle objects can be passed from one owner to another, like batons.
	 */
	typedef std::unique_ptr<engine> handle;

protected:
	virtual int do_run();

};

} // end namespace nitro

#endif // sentry
