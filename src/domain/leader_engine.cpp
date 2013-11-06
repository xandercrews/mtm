#define _INCLUDED_BY_NON_CLIENT_CODE
#include "domain/leader_engine.h"

namespace nitro {

leader_engine::leader_engine(int passive_port, int active_port) :
		engine(passive_port, active_port) {
}

leader_engine::~leader_engine() {
}

engine_handle leader_engine::make(int passive_port, int active_port) {
	return engine_handle(new leader_engine(passive_port, active_port));
}

bool register_leader_engine() {
	engine_factory & factory = engine_factory::singleton();
	return factory.register_ctor(leader_engine::make, false, NULL);
}

} // end namespace nitro
