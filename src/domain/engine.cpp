#include <stdexcept>
#include <thread>

#include "base/dbc.h"
#include "base/event_codes.h"
#include "base/guid.h"
#include "base/interp.h"
#include "base/netutil.h"
#include "base/xlog.h"

#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/engine.h"
#include "domain/event_codes.h"
#include "domain/msg.h"
#include "domain/worker_engine.h"
#include "domain/zmq_helpers.h"

#include "zeromq/include/zmq.hpp"

using std::mutex;
using std::lock_guard;

namespace nitro {

const char * const COORDINATION_TOPIC = events::catalog().get_topic(
		nitro::event_codes::NITRO_REQUEST_HELP);

// hardware_concurrency() can return 0 if unable to determine; assume at least 4
const unsigned MAX_HARDWARE_THREADS = std::max(4u, std::thread::hardware_concurrency());

engine::engine(cmdline const & cmdline) :
		ctx(_ctx), responder(sockets[ep_reqrep]), publisher(sockets[ep_pubsub]),
		_ctx(0), id(), linger(false) {

	auto reqrep_port = cmdline.get_option_as_int("--rrport", DEFAULT_REQREP_PORT);
	auto pubsub_port = cmdline.get_option_as_int("--psport", DEFAULT_PUBSUB_PORT);
	PRECONDITION(reqrep_port > 1024 && reqrep_port < 65536);
	PRECONDITION(pubsub_port > 1024 && pubsub_port < 65536);
	PRECONDITION(reqrep_port != pubsub_port);

	ports[ep_reqrep] = reqrep_port;
	ports[ep_pubsub] = pubsub_port;

	if (cmdline.has_flag("--linger")) {
		linger = true;
	}

	id = generate_guid();

    _ctx = zmq_ctx_new();
    sockets[ep_pubsub] = zmq_socket(_ctx, ZMQ_PUB);
    sockets[ep_reqrep] = zmq_socket(_ctx, ZMQ_REP);

    // Figure out what tcp endpoints we're going to be using.
    auto t1 = interp(TCP_BIND_ENDPOINT_PATTERN, pubsub_port);
    auto t2 = interp(TCP_BIND_ENDPOINT_PATTERN, reqrep_port);
    char const * firstaddr = nullptr;
    for (int i = 0; ; ++i) {
    	auto x = get_local_ipaddr(i);
    	if (x == nullptr) {
    		break;
    	}
    	if (firstaddr == nullptr || strcmp(firstaddr, "127.0.0.1") == 0) {
    		firstaddr = x;
    	}
    }
    endpoints[ep_pubsub][et_tcp] = interp(TCP_CONNECT_ENDPOINT_PATTERN,
    		firstaddr, pubsub_port);
    endpoints[ep_reqrep][et_tcp] = interp(TCP_CONNECT_ENDPOINT_PATTERN,
    		firstaddr, reqrep_port);

    // Bind to tcp.
   	zmq_bind_and_log(publisher, t1.c_str());
   	zmq_bind_and_log(responder, t2.c_str());
}

engine::~engine() {
	for (int pat = ep_pubsub; pat <= ep_reqrep; ++pat) {
		zmq_close_now(sockets[pat]);
	}
	if (ctx) {
		zmq_ctx_destroy(ctx);
	}
}

void engine::bind_after_ctor(char const * style) {

	auto pid = getpid();

    endpoints[ep_pubsub][et_ipc] = interp(IPC_PUBSUB_ENDPOINT_PATTERN, pid, style);
    endpoints[ep_reqrep][et_ipc] = interp(IPC_REQREP_ENDPOINT_PATTERN, pid, style);
    endpoints[ep_pubsub][et_inproc] = interp(INPROC_PUBSUB_ENDPOINT_PATTERN, style);
    endpoints[ep_reqrep][et_inproc] = interp(INPROC_REQREP_ENDPOINT_PATTERN, style);

    for (int pat = ep_pubsub; pat <= ep_reqrep; ++pat) {
    	for (int trans = et_ipc; trans <= et_inproc; ++trans) {
    		zmq_bind_and_log(sockets[pat], get_endpoint(pat, trans));
    	}
    }
}

char const * engine::get_id() const {
	return id.c_str();
}

bool engine::get_linger() const {
	return linger;
}

void engine::queue_for_send(void * socket, std::string const & msg) {
	lock_guard<mutex> lock(send_queue_mutex);
	send_queue.push({socket, msg});
}

void engine::send_queued() {
	lock_guard<mutex> lock(send_queue_mutex);
	while (!send_queue.empty()) {
		auto item = send_queue.front();
		send_full_msg(item.first, item.second);
		send_queue.pop();
	}
}

char const * engine::get_endpoint(int ep_pattern, int et_transport) const {
	PRECONDITION(ep_pattern >= ep_pubsub && ep_pattern <= ep_reqrep);
	PRECONDITION(et_transport >= et_tcp && et_transport <= et_inproc);
	return endpoints[ep_pattern][et_transport].c_str();
}

int engine::get_port(int ep_pattern) const {
	PRECONDITION(ep_pattern >= ep_pubsub && ep_pattern <= ep_reqrep);
	return ports[ep_pattern];
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
