#define _INCLUDED_BY_NON_CLIENT_CODE
#include "domain/follower_engine.h"

namespace nitro {

follower_engine::follower_engine(int server_port, int client_port,
		char const * transport) : engine(server_port, client_port, transport) {
}

follower_engine::~follower_engine() {
}

engine_handle follower_engine::make(int server_port, int client_port,
			char const * transport) {
	return engine_handle(new follower_engine(server_port, client_port, transport));
}

bool register_follower_engine() {
	engine_factory & factory = engine_factory::singleton();
	return factory.register_ctor(follower_engine::make, true, NULL);
}

} /* namespace nitro */
