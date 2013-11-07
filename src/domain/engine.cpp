#include <stdexcept>
#include <thread>

#include "base/dbc.h"
#include "base/event_ids.h"
#include "base/guid.h"
#include "base/interp.h"

#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/engine.h"
#include "domain/event_ids.h"
#include "domain/worker_engine.h"

#include "zeromq/include/zmq.hpp"

namespace nitro {

engine::engine(cmdline const & cmdline) :
		ctx(_ctx), responder(_responder), publisher(_publisher),
		reply_port(0), publish_port(0), id(), inproc_endpoint() {
	reply_port = cmdline.get_option_as_int("--replyport", DEFAULT_PASSIVE_PORT);
	publish_port = cmdline.get_option_as_int("--publishport", DEFAULT_ACTIVE_PORT);
	PRECONDITION(reply_port > 1024 && reply_port < 65536);
	PRECONDITION(publish_port > 1024 && publish_port < 65536);
	PRECONDITION(reply_port != publish_port);
	{
		char buf[GUID_BUF_LEN];
		generate_guid(buf, sizeof(buf));
		id = buf;
	}
	inproc_endpoint = interp("inproc://%1", id);
    _ctx = zmq_ctx_new();
    _publisher = zmq_socket(_ctx, ZMQ_PUB);
    int rc = zmq_bind(_publisher, inproc_endpoint.c_str());
    if (rc != 0) {
    	zmq_close(_publisher);
    	zmq_ctx_destroy(_ctx);
    	_ctx = 0;
    	throw ERROR_EVENT(errno);
    }
}

engine::~engine() {
	if (publisher) {
		zmq_close(publisher);
	}
	if (responder) {
		zmq_close(responder);
	}
	if (ctx) {
		zmq_ctx_destroy(ctx);
	}
}

char const * engine::get_id() const {
	return id.c_str();
}

char const * engine::get_inproc_endpoint() const {
	return inproc_endpoint.c_str();
}

int engine::get_reply_port() const {
	return reply_port;
}

int engine::get_publish_port() const {
	return publish_port;
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

engine::handle make_engine(cmdline const & cmdline) {
	bool worker_mode = cmdline.get_option("--workfor") != NULL;
	if (worker_mode) {
		return engine::handle(new worker_engine(cmdline));
	} else {
		return engine::handle(new coord_engine(cmdline));
	}
}



#if 0
int engine::run() {
#if 0
	start listening on reply_port

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
#endif

} // end namespace nitro
