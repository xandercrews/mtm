/*
 * engine.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#include <stdexcept>

#include "engine.h"

namespace mtm {

Engine::Engine() {
	// TODO Auto-generated constructor stub

}

Engine::~Engine() {
	// TODO Auto-generated destructor stub
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
