#define _INCLUDED_BY_NON_CLIENT_CODE
#include "domain/leader_engine.h"

namespace nitro {

leader_engine::leader_engine(int server_port, int client_port,
		char const * transport) : engine(server_port, client_port, transport) {
}

leader_engine::~leader_engine() {
}

engine_handle leader_engine::make(int server_port, int client_port,
			char const * transport) {
	return engine_handle(new leader_engine(server_port, client_port, transport));
}

bool register_leader_engine() {
	engine_factory & factory = engine_factory::singleton();
	return factory.register_ctor(leader_engine::make, false, NULL);
}

} // end namespace nitro
