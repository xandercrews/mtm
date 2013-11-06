#ifndef _DOMAIN_FOLLOWER_ENGINE_H_
#define _DOMAIN_FOLLOWER_ENGINE_H_

#ifndef _INCLUDED_BY_NON_CLIENT_CODE
#error "Do not include this file directly. Use engine_factory to make engines."
#endif

#include "domain/engine.h"

namespace nitro {

/**
 * The engine used when the app is in "follower" mode, waiting for instructions
 * from a leader.
 */
class follower_engine : public engine {

	follower_engine(int passive_port, int active_port);

public:
	virtual ~follower_engine();
	static engine_handle make(int passive_port, int active_port);

	virtual bool is_follower() const { return true; }
};

} // end namespace nitro

#endif // sentry
