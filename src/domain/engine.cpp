#include <stdexcept>
#include <thread>
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

void Engine::handle_ping_request(/*zmq::message_t const & msg*/) const {
	// send back a ping response
}

void Engine::handle_terminate_request(/*zmq::message_t const & msg*/) const {
	// shut down the program
}

void Engine::send_progress_report() const {
	const std::chrono::milliseconds DURATION(2000);
	while (true) {
		std::this_thread::sleep_for(DURATION);
		// zmq::send(msg with code = NITRO_BATCH_PROGRESS_REPORT
	}
}

int Engine::run() {
	PRECONDITION(listen_port != publish_port);

#if 0
	start listening on listen_port

	    If I get a json msg where the event code == NITRO_PING_REQUEST,
	    ... call handle_ping_request(). (see base/event_tuples.h to see how
	    ... the numeric value of NITRO_PING_REQUEST is built into a 32-bit
	    ... number).

	    If I get a json msg where the event code == NITRO_TERMINATE_REQUEST,
	    ... call handle_terminate_request();

	get ready to publish on publish_port



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
