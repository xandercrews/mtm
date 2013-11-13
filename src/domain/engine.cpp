#include <stdexcept>
#include <thread>

#include "base/dbc.h"
#include "base/event_codes.h"
#include "base/guid.h"
#include "base/interp.h"
#include "base/xlog.h"

#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/engine.h"
#include "domain/event_codes.h"
#include "domain/worker_engine.h"
#include "domain/zmq_helpers.h"

#include "zeromq/include/zmq.hpp"

namespace nitro {

const char * const COORDINATION_TOPIC = events::catalog().get_topic(
		nitro::event_codes::NITRO_REQUEST_HELP);

// hardware_concurrency() can return 0 if unable to determine; assume at least 4
const unsigned MAX_HARDWARE_THREADS = std::max(4u, std::thread::hardware_concurrency());

engine::engine(cmdline const & cmdline) :
		ctx(_ctx), responder(_responder), publisher(_publisher),
		reply_port(0), publish_port(0), _ctx(0), _responder(0), _publisher(0),
		id(), ipc_pub_endpoint(), tcp_pub_endpoint() {

	reply_port = cmdline.get_option_as_int("--replyport", DEFAULT_REPLY_PORT);
	publish_port = cmdline.get_option_as_int("--publishport", DEFAULT_PUBLISH_PORT);
	PRECONDITION(reply_port > 1024 && reply_port < 65536);
	PRECONDITION(publish_port > 1024 && publish_port < 65536);
	PRECONDITION(reply_port != publish_port);

	id = generate_guid();
    _ctx = zmq_ctx_new();
    _publisher = zmq_socket(_ctx, ZMQ_PUB);
    tcp_pub_endpoint = interp("tcp://*:%1", publish_port);
    zmq_bind_and_log(_publisher, tcp_pub_endpoint.c_str());
}

engine::~engine() {
	zmq_close_now(_publisher);
	zmq_close_now(_responder);
	if (ctx) {
		zmq_ctx_destroy(ctx);
	}
}

void engine::bind_publisher_to_ipc(char const * style) {
	ipc_pub_endpoint = interp(IPC_PUB_BINDING, getpid(), style);
	zmq_bind_and_log(_publisher, ipc_pub_endpoint.c_str());
}

char const * engine::get_id() const {
	return id.c_str();
}

char const * engine::get_subscribe_endpoint(char const * transport) const {
	PRECONDITION(transport);
	if (strcmp(transport, "tcp") == 0) {
		return tcp_pub_endpoint.c_str();
	}
	if (strcmp(transport, "ipc") == 0) {
		return ipc_pub_endpoint.c_str();
	}
	PRECONDITION("transport must be \"tcp\" or \"ipc\"." && false);
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

int engine::run() {
	int exit_code;
	try {
		xlog("Running engine %1", get_id());
		exit_code = do_run();
	} catch (std::exception const & e) {
		xlog(e.what());
		exit_code = 1;
	} catch (...) {
		xlog("Caught unrecognized exception.");
		exit_code = 1;
	}
	xlog("Returning exit code %1", exit_code);
	return exit_code;
}

} // end namespace nitro
