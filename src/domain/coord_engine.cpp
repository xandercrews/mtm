#include <algorithm>
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
			auto pargs = cmdline.get_positional_args();
			for (auto batch: pargs) {
				batches.push_back(batch);
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

coord_engine::assignment_t coord_engine::next_assignment() {
	const auto MAX_SIZE = 1000;
	assignment_t new_a;
	while (true) {
		if (!current_batch_file) {
			if (batches.empty()) {
				return new_a;
			} else {
				auto fl = new file_lines(batches.front().c_str());
				current_batch_file = std::unique_ptr<file_lines>(fl);
				batches.erase(batches.begin());
			}
		}
		while (true) {
			auto line = current_batch_file->next();
			if (line) {
				if (!new_a) {
					new_a = assignment_t(new stringlist_t);
				}
				new_a->push_back(line);
				if (new_a->size() >= MAX_SIZE) {
					return new_a;
				}
			} else {
				current_batch_file.reset();
			}
		}
	}
}

void coord_engine::prioritize(assignment_t & asgn) {
	// This is just a demo of doing a sort.
	struct priority_func {
		int operator ()(std::string const & a, std::string const & b) {
			// Here we could parse strings a and b and see if userprio was set,
			// and generate a sort key for each. In the long run, we probably
			// want to change the assignment_t datatype so it's not a list of
			// strings, but rather a list of parsed data that already has
			// sort key calculated.
			return a.size() - b.size();
		}
	};
	std::sort((*asgn).begin(), (*asgn).end(), priority_func());
}

void coord_engine::distribute(assignment_t & asgn) {
	auto host = hostlist.begin();
    for (auto cmd : *asgn) {
		auto endpoint = interp("tcp://%1", *host);
		void * requester;
    	try {
			requester = zmq_socket(ctx, ZMQ_REQ);
			int rc;
			tryz(zmq_connect(requester, endpoint.c_str()));
			auto msg = serialize_msg(NITRO_HERE_IS_ASSIGNMENT, cmd);
			send_full_msg(requester, msg);
			// Process ACK
			receive_full_msg(requester);
			++host;
			if (host == hostlist.end()) {
				host = hostlist.begin();
			}
			zmq_close(requester);
    	} catch (error_event const & e) {
    		zmq_close(requester);
    	}
    }
}

int coord_engine::do_run() {
	// This is totally the wrong way to do dispatch of assignments. I'm
	// completely abusing zmq by short-circuiting its own fair share routing
	// and by creating and destroying sockets right and left. I've only done
	// it this way to get some logic running that I can improve incrementally.

	enroll_workers(NITRO_REQUEST_HELP);

	while (true) {
		auto assignment = next_assignment();
		if (!assignment) {
			break;
		}
		prioritize(assignment);
		// who's not busy?
		distribute(assignment);
	}

	enroll_workers(NITRO_TERMINATE_REQUEST);
	this_thread::sleep_for(chrono::milliseconds(100));
	int linger = 0;
	zmq_setsockopt(requester, ZMQ_LINGER, &linger, sizeof(linger));
	zmq_close(requester);
	xlog("Completed all batches.");

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
