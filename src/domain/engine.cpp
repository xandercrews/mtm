#include <stdexcept>

#include "domain/engine.h"

namespace nitro {

Engine::Engine() {

}

Engine::~Engine() {
}

void Engine::set_port(int port) {

}

void Engine::set_slave_mode(bool value) {

}

int Engine::run() {

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
