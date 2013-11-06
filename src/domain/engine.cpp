#include <stdexcept>
#include <thread>
#include <vector>
#include <zmq.h>

#include "base/dbc.h"
#include "base/event_ids.h"
#include "base/interp.h"

#include "domain/engine.h"
#include "domain/event_ids.h"
#include "domain/cmdline.h"

namespace nitro {

engine::engine(int s_port, int c_port, char const * trans) :
		server_port(s_port), client_port(c_port), transport(trans), zmq_ctx(0),
		zmq_server_socket(0), zmq_client_socket(0) {
	PRECONDITION(server_port > 1024 && server_port < 65536);
	PRECONDITION(client_port > 1024 && client_port < 65536);
	PRECONDITION(server_port != client_port);
	PRECONDITION(trans && *trans);
	zmq_ctx = zmq_ctx_new();
	zmq_server_socket = zmq_socket(zmq_ctx, ZMQ_REP);
	int rc = zmq_bind(zmq_server_socket, interp(
			"%1{transport}://*:%2{port}", transport, server_port).c_str());
	// TODO: use scope guard to clean up here.
	if (rc) {
		zmq_ctx_destroy(zmq_ctx);
		zmq_ctx = 0;
	} else {
		zmq_client_socket = zmq_socket(zmq_ctx, ZMQ_REP);
	}
}

engine::~engine() {
	if (zmq_client_socket) {
		zmq_close(zmq_client_socket);
	}
	if (zmq_server_socket) {
		zmq_close(zmq_server_socket);
	}
	if (zmq_ctx) {
		zmq_ctx_destroy(zmq_ctx);
	}
}

int engine::get_server_port() const {
	return server_port;
}

int engine::get_client_port() const {
	return client_port;
}

void engine::handle_ping_request(/*zmq::message_t const & msg*/) const {
	// send back a ping response
}

void engine::handle_terminate_request(/*zmq::message_t const & msg*/) const {
	// shut down the program
}

void send_progress_report_thread_main() {
	const std::chrono::milliseconds DURATION(2000);
	while (true) {
		std::this_thread::sleep_for(DURATION);
		// zmq::send(msg with code = NITRO_BATCH_PROGRESS_REPORT
	}
}

int engine::run() {

#if 0
	start listening on listen_port

	    If I get a json msg where the event code == NITRO_PING_REQUEST,
	    ... call handle_ping_request(). (see base/event_tuples.h to see how
	    ... the numeric value of NITRO_PING_REQUEST is built into a 32-bit
	    ... number).

	    If I get a json msg where the event code == NITRO_TERMINATE_REQUEST,
	    ... call handle_terminate_request();

	get ready to publish on publish_port (accept subscriber requests)

	   call send_progress_report_thread_main();

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
	return 0;
}

engine_factory & engine_factory::singleton() {
	static engine_factory the_factory;
	return the_factory;
}

struct ctor_info {
	engine_factory::engine_ctor ctor;
	std::string tag;
	bool follow_mode;
};

typedef std::vector<ctor_info> ctor_infos;

struct engine_factory::data_t {
	ctor_infos infos;
};

engine_factory::engine_factory() : data(new data_t) {
}

engine_factory::~engine_factory() {
	delete data;
}

engine_handle engine_factory::make(cmdline const & cmdline, char const * tag)
		const {
	bool follow_mode = cmdline.has_flag("--follow");
	int listen_port = cmdline.get_option_as_int("--listenport", DEFAULT_LISTEN_PORT);
	int talk_port = cmdline.get_option_as_int("--talkport", DEFAULT_TALK_PORT);
	char const * transport = "tcp"; //TODO: figure out a better way
	for (auto i: data->infos) {
		if (i.follow_mode == follow_mode) {
			if (tag == NULL || *tag == 0 || i.tag == tag) {
				return i.ctor(listen_port, talk_port, transport);
			}
		}
	}
	return NULL;
}

bool engine_factory::register_ctor(engine_ctor ctor, bool follow_mode,
			char const * tag) {
	for (auto i: data->infos) {
		if (i.ctor == ctor) {
			return false;
		}
	}
	ctor_info ci;
	ci.ctor = ctor;
	ci.follow_mode = follow_mode;
	ci.tag = tag ? tag : "";
	data->infos.push_back(ci);
	return true;
}

// Force helper_engine and leader_engine to be linked. Tests or other code
// can add others if it likes, but these two engines always need to be
// available.
extern bool register_leader_engine();
extern bool register_follower_engine();

bool rle = register_leader_engine();
bool rfe = register_follower_engine();

} // end namespace nitro
