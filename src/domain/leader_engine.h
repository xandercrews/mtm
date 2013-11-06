#ifndef _DOMAIN_LEADER_ENGINE_H_
#define _DOMAIN_LEADER_ENGINE_H_

#ifndef _INCLUDED_BY_NON_CLIENT_CODE
#error "Do not include this file directly. Use engine_factory to make engines."
#endif

#include "domain/engine.h"

namespace nitro {

/**
 * The engine used when the app is in "leader" mode, giving instructions to
 * followers.
 */
class leader_engine : public engine {

	leader_engine(int listen_port, int talk_port, char const * transport);

public:
	virtual ~leader_engine();

	static engine_handle make(int listen_port, int talk_port,
			char const * transport);

	virtual bool is_follower() const { return false; }
};

} // end namespace nitro

#endif // sentry
