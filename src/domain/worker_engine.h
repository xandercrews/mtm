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
};

} // end namespace nitro

#endif // sentry
