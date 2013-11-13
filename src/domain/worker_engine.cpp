#include <atomic>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <map>
#include <list>
#include <chrono>

#include <string.h>

#include "base/dbc.h"
#include "base/file_lines.h"
#include "base/strutil.h"
#include "base/xlog.h"

#include "domain/assignment.h"
#include "domain/cmdline.h"
#include "domain/worker_engine.h"
#include "domain/event_codes.h"
#include "domain/msg.h"
#include "domain/zmq_helpers.h"

#include "json/json.h"

#include "zeromq/include/zmq.h"

using std::string;
using std::list;
using std::thread;
using std::mutex;
using std::map;
using std::atomic;
using std::lock_guard;
using std::chrono::milliseconds;
using std::chrono::high_resolution_clock;

using namespace nitro::event_codes;

namespace nitro {

void default_thread_main(worker_engine * we, char const * cmdline) {
	worker_engine::notifier notifier(*we);
	FILE * f = popen(cmdline, "r");
	if (f) {
		char buf[1024];
		while (fgets(buf, sizeof(buf), f) != NULL) {
			;
		}
		pclose(f);
	}
}

thread default_launch_func(worker_engine * we, char const * cmdline) {
	return thread(default_thread_main, we, cmdline);
}

typedef std::list<thread> threadlist_t;
typedef std::list<assignment::handle> assignmentlist_t;

struct worker_engine::data_t {
	void * subscriber;
	threadlist_t threadlist;
	mutex tlist_mutex;
	atomic<uint> active_thread_count;
	assignmentlist_t assignmentlist;
	mutex alist_mutex;
	string workfor;
	launch_func launcher;
	bool enrolled;

	data_t() :
			subscriber(0), threadlist(), active_thread_count(0), launcher(0), enrolled(
					false) {
	}
};

worker_engine::notifier::~notifier() {
	we.notify_thread_complete(std::this_thread::get_id());
}

void worker_engine::notify_thread_complete(thread::id tid) {
	lock_guard < mutex > lock(data->tlist_mutex);
	threadlist_t & tlist = data->threadlist;
	for (auto i = tlist.begin(); i != tlist.end(); ++i) {
		if (i->get_id() == tid) {
			tlist.erase(i);
			data->active_thread_count--;
			return;
		}
	}
}

worker_engine::worker_engine(cmdline const & cmdline) :
		engine(cmdline), data(new data_t) {

	auto subscriber = zmq_socket(ctx, ZMQ_SUB);
	if (subscriber) {

		data->subscriber = subscriber;
		zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, COORDINATION_TOPIC,
				strlen(COORDINATION_TOPIC));
		bind_publisher_to_ipc("wpub");

		auto wf = cmdline.get_option("--workfor", "");
		auto proto = strstr(wf, "://");
		if (proto) {
			wf = proto + 3;
		}
		if (*wf) {
			data->workfor = wf;
			auto wfport = strrchr(data->workfor.c_str(), ':');
			if (!wfport) {
				data->workfor += interp(":%1", DEFAULT_PUBLISH_PORT);
			}
			auto endpoint = interp("tcp://%1", data->workfor);
			zmq_connect_and_log(subscriber, endpoint.c_str());

#if 0
			auto eth = cmdline.get_option("--interface",
					DEFAULT_MULTICAST_INTERFACE);
			// TODO: use different ip addr
			endpoint = interp("epgm://%1;127.0.0.1", eth);
			zmq_connect_and_log(subscriber, endpoint.c_str());
#endif
		}
	}
}

const char * worker_engine::get_workfor() const {
	return data->workfor.c_str();
}

worker_engine::~worker_engine() {
	zmq_close_now(data->subscriber);
	delete data;
}

worker_engine::launch_func worker_engine::get_launch_func() const {
	return data->launcher;
}

void worker_engine::set_launch_func(launch_func value) {
	data->launcher = value ? value : default_launch_func;
}

assignment * worker_engine::get_current_assignment() const {
	return data->assignmentlist.empty() ? 0 : data->assignmentlist.front().get();
}

void worker_engine::start_more_tasks() {
	// TODO: make desired_busy_threads a configurable member.
	auto desired_busy_threads = MAX_HARDWARE_THREADS;
	static bool logged_thread_count = false;
	if (!logged_thread_count) {
		logged_thread_count = true;
		xlog("worker_engine will try to keep %1 threads busy.",
				desired_busy_threads);
	}
	auto atc = data->active_thread_count.load();
	if (atc < desired_busy_threads) {
		auto asgn = get_current_assignment();
		if (asgn) {
			assignment::tasklist_t const & readylist =
					asgn->get_list_by_status(task_status::ts_ready);
			for (auto i = readylist.begin(); i != readylist.end(); ++i) {
				task const & t = **i;
				asgn->activate_task(t.get_id());
				lock_guard<mutex> lock(data->tlist_mutex);
				data->threadlist.push_back(data->launcher(this, t.get_cmdline()));
				if (++atc == desired_busy_threads) {
					break;
				}
			}
		}
	}
}

void worker_engine::respond_to_help_request(void * socket) {
	if (!data->enrolled) {
		data->enrolled = true;
		send_full_msg(socket, serialize_msg(NITRO_AFFIRM_HELP));
	} else {
		send_full_msg(socket,
				serialize_msg(NITRO_DENY_HELP_1REASON, "not yet enrolled"));
	}
}

void worker_engine::accept_assignment(assignment * asgn) {
	lock_guard < mutex > lock(data->alist_mutex);
	data->assignmentlist.push_back(assignment::handle(asgn));
}

void worker_engine::respond_to_assignment(void * socket,
		Json::Value const & json) {
	string txt;
	if (data->enrolled) {
		auto aid = json["body"]["assignment"]["id"];
		auto lines = json["body"]["assignment"]["lines"];
		assignment * asgn = new assignment(aid.asCString(), lines.asCString());
		accept_assignment(asgn);
		txt = serialize_msg(NITRO_ACCEPT_ASSIGNMENT);
	} else {
		txt = serialize_msg(NITRO_REJECT_ASSIGNMENT_1REASON,
				"not yet enrolled");
	}
	send_full_msg(socket, txt);
}

void worker_engine::report_status() {
	auto asgn = get_current_assignment();
	if (asgn) {
		auto txt = asgn->get_status_msg();
		send_full_msg(publisher, txt);
	} else {
		// TODO: REPORT IDLE
	}
}

int worker_engine::do_run() {

	const int ITEM_COUNT = 2;
	zmq_pollitem_t items[ITEM_COUNT];
	items[0].socket = responder;
	items[0].events = ZMQ_POLLIN;
	items[1].socket = data->subscriber;
	items[1].events = ZMQ_POLLIN;

	const auto REPORTING_INTERVAL = milliseconds(5000);
	high_resolution_clock clock;
	auto last_status_report = clock.now() - REPORTING_INTERVAL;

	while (true) {

		start_more_tasks();

		auto time_since_last_status_report = clock.now() - last_status_report;
		if (time_since_last_status_report > REPORTING_INTERVAL) {
			report_status();
			last_status_report = clock.now();
		}

		int rc = zmq_poll(items, 1, 25);
		if (rc) {

			// Process all messages that have accumulated.
			while (true) {

				for (int i = 0; i < ITEM_COUNT; ++i) {

					if (items[i].revents == 0) {
						continue;
					}

					auto txt = receive_full_msg(items[i].socket);
					if (txt.empty()) {
						continue;
					}

					void * socket = items[i].socket;

#define IF_SOCKET_N_HANDLE(num, block) \
						if (i == num) { block; } \
						else { xlog("Can't handle msg on socket %1", num); } break

					Json::Value json;
					if (deserialize_msg(txt, json)) {
						auto code = json["body"]["code"].asInt();
						switch (code) {
						case NITRO_REQUEST_HELP:
							IF_SOCKET_N_HANDLE(0,
									respond_to_help_request(socket))
;							case NITRO_HERE_IS_ASSIGNMENT:
							IF_SOCKET_N_HANDLE(0, respond_to_assignment(socket, json));
							default:
							xlog("Unrecognized message %1 (%2)",
									events::get_std_id_repr(code),
									events::catalog().get_msg(code));
						}
					}
				}
				// See if there are other messages that need processing
				// before we go back to sleep.
				rc = zmq_poll(items, ITEM_COUNT, 0);
				if (rc == 0) {
					break;
				}
			}
		}
	}

	return 0;
}

} // end namespace nitro
