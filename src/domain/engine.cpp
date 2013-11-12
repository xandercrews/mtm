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

#include "zeromq/include/zmq.hpp"

namespace nitro {

const std::string engine::_subscription = "nitro";

engine::engine(cmdline const & cmdline) :
		ctx(_ctx), responder(_responder), publisher(_publisher),
		reply_port(0), publish_port(0), _ctx(0), _responder(0), _publisher(0),
		id(), inproc_endpoint(), tcp_endpoint() {
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
    _ctx = zmq_ctx_new();
    _publisher = zmq_socket(_ctx, ZMQ_PUB);
    // TODO: improve with scopeguard
	inproc_endpoint = interp("inproc://%1", id);
    int rc = zmq_bind(_publisher, inproc_endpoint.c_str());
    if (rc == 0) {
    	tcp_endpoint = interp("tcp://127.0.0.1:%1", publish_port);
//    	rc = zmq_bind(_publisher, tcp_endpoint.c_str());
    }
    if (rc != 0) {
    	zmq_close(_publisher);
    	_publisher = 0;
    	zmq_ctx_destroy(_ctx);
    	_ctx = 0;
    	throw ERROR_EVENT(errno);
    }
}

engine::~engine() {
	int linger = 0;
	if (_publisher) {
		zmq_setsockopt(_publisher, ZMQ_LINGER, &linger, sizeof(linger));
		zmq_close(_publisher);
	}
	if (_responder) {
		zmq_setsockopt(_responder, ZMQ_LINGER, &linger, sizeof(linger));
		zmq_close(_responder);
	}
	if (ctx) {
		zmq_ctx_destroy(ctx);
	}
}

char const * engine::get_id() const {
	return id.c_str();
}

char const * engine::get_subscribe_endpoint(char const * transport) const {
	PRECONDITION(transport);
	if (strcmp(transport, "tcp") == 0) {
		return tcp_endpoint.c_str();
	}
	if (strcmp(transport, "inproc") == 0) {
		return inproc_endpoint.c_str();
	}
	PRECONDITION("transport must be \"tcp\" or \"inproc\"." && false);
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

#if 0
int engine::run() {
	start listening on reply_port

	    If I get a json msg where the event code == NITRO_PING_REQUEST,
	    ... call handle_ping_request(). (see base/event_tuples.h to see how
	    ... the numeric value of NITRO_PING_REQUEST is built into a 32-bit
	    ... number).

	    If I get a json msg where the event code == NITRO_TERMINATE_REQUEST,
	    ... call handle_terminate_request();

	get ready to publish on publish_port (accept subscriber requests)

	   call send_progress_report_thread_main();

	return 0;
}
#endif

} // end namespace nitro
