#include <stdexcept>
#include <thread>
#include <zmq.h>

#include "base/dbc.h"
#include "base/event_ids.h"
#include "base/interp.h"
#include "domain/engine.h"
#include "domain/event_ids.h"

namespace nitro {

struct engine::data_t {
	int publish_port;
	int listen_port;
	bool follow_mode;
	void * context;
	void * reqrep_socket;
	std::string transport;

	data_t() : publish_port(47000),	listen_port(47001), follow_mode(false),
			context(0), reqrep_socket(0) {
	}

	~data_t() {
		if (reqrep_socket) {
			zmq_close(reqrep_socket);
		}
		if (context) {
			zmq_ctx_destroy(context);
		}
	}

	inline bool is_running() const {
		return context != 0;
	}
};

engine::engine() : data(new data_t) {
}

engine::~engine() {
	delete data;
}

void engine::init_zmq() {
	PRECONDITION(!data->is_running());
	auto ctx = zmq_ctx_new();
	data->reqrep_socket = zmq_socket(data->context, ZMQ_REP);
	int rc = zmq_bind(data->reqrep_socket,
			interp("%1{transport}://*:%2{port}", data->transport,
					data->listen_port).c_str());
	CHECK(rc == 0);
	data->context = ctx;
}

void engine::set_publish_port(int port) {
	PRECONDITION(!data->is_running());
	PRECONDITION(port > 1024 && port < 65536);
	data->publish_port = port;
}

inline int engine::get_publish_port() const {
	return data->publish_port;
}

void engine::set_listen_port(int port) {
	PRECONDITION(!data->is_running());
	PRECONDITION(port > 1024 && port < 65536);
}

inline int engine::get_listen_port() const {
	return data->listen_port;
}

void engine::set_follow_mode(bool value) {
	PRECONDITION(!data->is_running());
	data->follow_mode = value;
}

inline bool engine::get_follow_mode() const {
	return data->follow_mode;
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
	PRECONDITION(data->listen_port != data->publish_port);

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

} /* namespace mtm */
