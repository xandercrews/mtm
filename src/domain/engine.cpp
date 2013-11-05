#include <stdexcept>
#if 0
#include <zmq.hpp>
#endif

#include "base/dbc.h"
#include "domain/engine.h"

namespace nitro {

Engine::Engine() : publish_port(47000),	listen_port(47001), follow_mode(false),
		running(false) {
}

Engine::~Engine() {
}

void Engine::set_publish_port(int port) {
	PRECONDITION(!running);
	PRECONDITION(port > 1024 && port < 65536);
	publish_port = port;
}

inline int Engine::get_publish_port() const {
	return publish_port;
}

void Engine::set_listen_port(int port) {
	PRECONDITION(!running);
	PRECONDITION(port > 1024 && port < 65536);
}

inline int Engine::get_listen_port() const {
	return listen_port;
}

void Engine::set_follow_mode(bool value) {
	PRECONDITION(!running);
	follow_mode = value;
}

inline bool Engine::get_follow_mode() const {
	return follow_mode;
}

int Engine::run() {
	PRECONDITION(listen_port != publish_port);

#if 0
	start listening
#endif

#if 0
	initialize(); // prove we can communicate
	start_listening();
	while (more_files) {
		try {
			Batch batch(file);
			Chunk = get_chunk();
			chunk.prioritize();
			chunk.find_eligible_jobs();
			chunk.distribute();
		} catch (std::runtime_error const & e) {

		}
	}
#endif
}

} /* namespace mtm */
