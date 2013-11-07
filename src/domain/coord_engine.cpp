#include <condition_variable>
#include <thread>

#include "base/dbc.h"
#include "base/file_lines.h"
#include "base/strutil.h"
#include "base/xlog.h"

#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/event_ids.h"
#include "domain/msg.h"

#include "zeromq/include/zmq.h"

using namespace std;
using namespace nitro::event_ids;

namespace nitro {

coord_engine::coord_engine(cmdline const & cmdline) :
		engine(cmdline), requester(0) {

	auto exec_host = cmdline.get_option("--exechost");
	if (!exec_host) {
		exec_host = getenv("exec_host");
	}
	if (exec_host) {
		try {
			file_lines fl(exec_host);
			while (true) {
				auto line = fl.next();
				if (line == nullptr) {
					break;
				}
#if 0
				string ln(line);
				split(ln, ',', hostlist);
#else
				hostlist.push_back(line);
#endif

			}
			return;
		} catch (error_event const & e) {
			// Must not be a file. Try raw list.
#if 0
			string eh(exec_host);
			split(trim(eh), ',', hostlist);
#else
			hostlist.push_back(exec_host);
#endif
		}
	}
}

coord_engine::~coord_engine() {
}

#define tryz(expr) rc = expr; if (rc) throw ERROR_EVENT(errno)

void coord_engine::enroll_workers(int eid) {
#if 1
	if (hostlist.empty()) {
		hostlist.push_back("localhost:36000");
	}
#endif
	if (!hostlist.empty()) {
		try {
			for (auto host: hostlist) {
				requester = zmq_socket(ctx, ZMQ_REQ);
				string endpoint = interp("tcp://%1", host);
				int rc;
				tryz(zmq_connect(requester, endpoint.c_str()));
				string msg = serialize_msg(eid);
				send_full_msg(requester, msg);
				auto response = receive_full_msg(requester);
				// TODO: check response
				xlog("Enrolled %1.", endpoint);
				zmq_close(requester);
			}
		} catch (error_event const & e) {
			zmq_close(requester);
			throw;
		}
	}
}

int coord_engine::do_run() {
	// This is totally the wrong way to do dispatch of assignments. I'm
	// completely abusing zmq by short-circuiting its own fair share routing
	// and by creating and destroying sockets right and left. I've only done
	// it this way to get some logic running that I can improve incrementally.

	enroll_workers(NITRO_REQUEST_HELP);

	auto host = hostlist.begin();
    for (int i = 0; i < 1000; ++i) {
		auto cmd = interp("fake commandline %1", i + 1);
		auto endpoint = interp("tcp://%1", *host);
		void * requester;
    	try {
			requester = zmq_socket(ctx, ZMQ_REQ);
			int rc;
			tryz(zmq_connect(requester, endpoint.c_str()));
			auto msg = serialize_msg(NITRO_HERE_IS_ASSIGNMENT, cmd);
			send_full_msg(requester, msg);
			++host;
			if (host == hostlist.end()) {
				host = hostlist.begin();
			}
			zmq_close(requester);
    	} catch (error_event const & e) {
    		zmq_close(requester);
    	}
    }

	enroll_workers(NITRO_TERMINATE_REQUEST);
	this_thread::sleep_for(chrono::milliseconds(100));
	zmq_setsockopt(requester, ZMQ_LINGER, 0, sizeof(0));
	zmq_close(requester);
    return 0;
}

#if 0
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
} // end namespace nitro
