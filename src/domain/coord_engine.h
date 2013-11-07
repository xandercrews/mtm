#ifndef _DOMAIN_LEADER_ENGINE_H_
#define _DOMAIN_LEADER_ENGINE_H_

#include "domain/engine.h"

namespace nitro {

/**
 * The engine used when the app is in "coordinator" mode, giving instructions to
 * workers.
 */
class coord_engine : public engine {
public:
	coord_engine(int reply_port, int publish_port);
	virtual ~coord_engine();
};

} // end namespace nitro

#endif // sentry
