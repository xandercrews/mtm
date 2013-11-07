#ifndef _DOMAIN_LEADER_ENGINE_H_
#define _DOMAIN_LEADER_ENGINE_H_

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

	typedef std::vector<std::string> hostlist_t;
	hostlist_t const & get_hostlist() const;

private:
	hostlist_t hostlist;
};

} // end namespace nitro

#endif // sentry
