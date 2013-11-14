#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <thread>

#include <string.h>
#include <unistd.h>

#include "base/dbc.h"
#include "base/file_lines.h"
#include "base/strutil.h"
#include "base/xlog.h"

#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/event_codes.h"
#include "domain/msg.h"
#include "domain/zmq_helpers.h"

#include "json/json.h"

#include "zeromq/include/zmq.h"

using std::string;
using std::chrono::milliseconds;

using namespace nitro::event_codes;

namespace nitro {

struct coord_engine::data_t {

	std::unique_ptr<file_lines> current_batch_file;
	stringlist_t hostlist;
	stringlist_t batches;
	void * requester;
	stringlist_t workers;

	data_t() :
			requester(0) {
	}
};

coord_engine::coord_engine(cmdline const & cmdline) :
		engine(cmdline), data(new data_t) {

	init_hosts(cmdline);

	bind_after_ctor("c");

	// bind socket for remote connections
	auto iface = cmdline.get_option("--interface", DEFAULT_MULTICAST_INTERFACE);
	// TODO: figure out what value to use for ip addr on next line...
	auto endpoint = interp("epgm://%1;239.192.1.1:%2", iface,
			get_publish_port());
	zmq_bind_and_log(publisher, endpoint.c_str());

	reporter_port = cmdline.get_option_as_int("--reporter",
			DEFAULT_REPORTER_PORT);
}

coord_engine::~coord_engine() {
	delete data;
}

void coord_engine::enroll_workers_multi(int eid) {
	static const string cfm_port = "50000";

	// Clear workers list
	data->workers.clear();

	string json_msg = serialize_msg(eid, cfm_port);
	zmq_msg_t msg, msg_copy;

	// Create 2 copy of message
	auto topic_len = strlen(COORDINATION_TOPIC);
	auto total_len = json_msg.size() + topic_len;
	zmq_msg_init_size(&msg, total_len);
	zmq_msg_init_size(&msg_copy, total_len);
	memcpy(zmq_msg_data(&msg), COORDINATION_TOPIC, topic_len);
	memcpy((char *) zmq_msg_data(&msg) + topic_len, json_msg.c_str(),
			json_msg.size());
	zmq_msg_copy(&msg_copy, &msg);

	// Send messages
	int rc = zmq_sendmsg(publisher, &msg, 0);
	xlog("message sending: %1", rc);

	// Wait for the workers responses
	void *cfm = zmq_socket(ctx, ZMQ_PULL);
	if (cfm) {
		string endpoint = interp("tcp://*:", cfm_port);
		zmq_bind(cfm, endpoint.c_str());

		zmq_pollitem_t items[1];
		items[0].socket = cfm;
		items[0].events = ZMQ_POLLIN;

		// Stop wating workers if no connection for 5 sec
		xlog("waiting confirmations from workers...");
		while ((rc = zmq_poll(items, sizeof(items) / sizeof(items[0]), 5000))
				> 0) {
			for (unsigned i = 0; i < sizeof(items) / sizeof(items[0]);
					i++) {
				if (items[i].revents & ZMQ_POLLIN) {
					Json::Value root;
					string json = receive_full_msg(items[i].socket);

					if (deserialize_msg(json, root)) {
						data->workers.push_back(
								root["body"]["message"].asCString());
						xlog("Got response from worker: %1",
								root["body"]["message"].asCString());
					}
				}
			}
		}

		zmq_close_now(cfm);
		xlog("... done waiting confirmations from workers");

		// TODO: connect directly to missed workerks
	}
}

#define tryz(expr) rc = expr; if (rc) throw ERROR_EVENT(errno)

void coord_engine::enroll_workers(int eid) {
#if 1
	if (data->hostlist.empty()) {
		data->hostlist.push_back("localhost:36000");
	}
#endif
	if (!data->hostlist.empty()) {
		try {
			for (auto host : data->hostlist) {
				data->requester = zmq_socket(ctx, ZMQ_REQ);
				auto endpoint = interp("tcp://%1", host);
				zmq_connect_and_log(data->requester, endpoint.c_str());
				auto msg = serialize_msg(eid);
				send_full_msg(data->requester, msg);
				auto response = receive_full_msg(data->requester);
				// TODO: check response
				xlog("Enrolled %1.", endpoint);
				zmq_close(data->requester);
			}
		} catch (error_event const & e) {
			zmq_close(data->requester);
			throw;
		}
	}
}

coord_engine::assignment_t coord_engine::next_assignment() {
	const auto MAX_SIZE = 1000;
	assignment_t new_a;
	while (true) {
		if (!data->current_batch_file) {
			if (data->batches.empty()) {
				return new_a;
			} else {
				auto fl = new file_lines(data->batches.front().c_str());
				data->current_batch_file = std::unique_ptr < file_lines > (fl);
				data->batches.erase(data->batches.begin());
			}
		}
		while (true) {
			auto line = data->current_batch_file->next();
			if (line) {
				if (!new_a) {
					new_a = assignment_t(new stringlist_t);
				}
				new_a->push_back(line);
				if (new_a->size() >= MAX_SIZE) {
					return new_a;
				}
			} else {
				data->current_batch_file.reset();
			}
		}
	}
}

void coord_engine::prioritize(assignment_t & asgn) {
	struct priority_func {
		int operator ()(string const & a, string const & b) {
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
	auto host = data->hostlist.begin();
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
			if (host == data->hostlist.end()) {
				host = data->hostlist.begin();
			}
			zmq_close(requester);
		} catch (error_event const & e) {
			zmq_close(requester);
		}
	}
}

void coord_engine::progress_reporter() {
	int rc = -1;

	// Socket for worker control
	void *reporter = zmq_socket(ctx, ZMQ_PUB);
	if (reporter) {
		auto endpoint = interp("tcp://*:%1", reporter_port);
		rc = zmq_bind(reporter, endpoint.c_str());
		xlog("reporter bind to socket %1 result %2", reporter_port, rc);
	}

	xlog("Start sending reports each second");

	while (report_progress && !rc) {
		string progress = "Progress(50%): So far done 500 jobs from 1000";
		auto msg = serialize_msg(NITRO_BATCH_PROGRESS_REPORT, progress);
		send_full_msg(reporter, msg);
		sleep(1);
	}

	if (reporter) {
		zmq_close(reporter);
	}

	xlog("Before report thread exit");
}

int coord_engine::do_run() {

	report_progress = true;
	std::thread t1(&coord_engine::progress_reporter, this);

	// This is totally the wrong way to do dispatch of assignments. I'm
	// completely abusing zmq by short-circuiting its own fair share routing
	// and by creating and destroying sockets right and left. I've only done
	// it this way to get some logic running that I can improve incrementally.

	enroll_workers_multi(NITRO_REQUEST_HELP);
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
	std::this_thread::sleep_for(milliseconds(100));
	int linger = 0;
	zmq_setsockopt(data->requester, ZMQ_LINGER, &linger, sizeof(linger));
	zmq_close(data->requester);
	xlog("Completed all batches.");

	report_progress = false;
	t1.join();

	return 0;
}

void coord_engine::init_hosts(cmdline const & cmdline) {
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
				data->hostlist.push_back(line);
#endif

			}
			auto pargs = cmdline.get_positional_args();
			for (auto batch : pargs) {
				data->batches.push_back(batch);
			}
			return;
		} catch (error_event const & e) {
			// Must not be a file. Try raw list.
#if 0
			string eh(exec_host);
			split(trim(eh), ',', hostlist);
#else
			data->hostlist.push_back(exec_host);
#endif
		}
	}
}

} // end namespace nitro
