#define _INCLUDED_BY_NON_CLIENT_CODE
#include "domain/follower_engine.h"

namespace nitro {

follower_engine::follower_engine(int passive_port, int active_port) :
		engine(passive_port, active_port) {
}

follower_engine::~follower_engine() {
}

engine_handle follower_engine::make(int passive_port, int active_port) {
	return engine_handle(new follower_engine(passive_port, active_port));
}

bool register_follower_engine() {
	engine_factory & factory = engine_factory::singleton();
	return factory.register_ctor(follower_engine::make, true, NULL);
}

} /* namespace nitro */
