#include <atomic>
#include <chrono>
#include <list>
#include <map>
#include <mutex>
#include <thread>

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

void default_thread_main(worker_engine & we, char const * cmdline) {
	worker_engine::notifier notifier(we);
	FILE * f = popen(cmdline, "r");
	if (f) {
		char buf[1024];
		while (fgets(buf, sizeof(buf), f) != NULL) {
			;
		}
		pclose(f);
	}
}

thread * default_launch_func(worker_engine & we, char const * cmdline) {
	return new thread(default_thread_main, std::ref(we), cmdline);
}

struct thread_task_pair {
	mutable thread * associated_thread;
	task const * associated_task;
	thread_task_pair(thread * th, task const * taskref) :
			associated_thread(th), associated_task(taskref) {
	}
	~thread_task_pair() {
		delete associated_thread;
	}
	// Implement move semantics, so that if you assign or copy construct one
	// of these from another, state is transferred and the old one becomes a
	// shell with no responsibilities.
	thread_task_pair(thread_task_pair const & other) :
			associated_thread(nullptr) {
		this->operator =(other);
	}
	thread_task_pair & operator =(thread_task_pair const & rhs) {
		if (associated_thread) {
			delete associated_thread;
		}
		associated_thread = rhs.associated_thread;
		associated_task = rhs.associated_task;
		rhs.associated_thread = nullptr;
		return *this;
	}
};

typedef std::map<thread::id, thread_task_pair> threadmap_t;
typedef std::queue<assignment::handle> asgn_queue_t;

struct worker_engine::data_t {
	void * subscriber;
	threadmap_t threadmap;
	mutex tmap_mutex;
	atomic<uint> active_thread_count;
	asgn_queue_t asgn_queue;
	mutex aqueue_mutex;
	string workfor;
	launch_func launcher;
	bool enrolled;

	data_t() :
			subscriber(0), threadmap(), active_thread_count(0), launcher(0),
			enrolled(false) {
	}
};

worker_engine::notifier::~notifier() {
	we.notify_thread_complete(std::this_thread::get_id());
}

void worker_engine::notify_thread_complete(thread::id tid) {
	task::id_type task_id_to_complete = UINT64_MAX;
	{
		lock_guard<mutex> lock(data->tmap_mutex);
		auto i = data->threadmap.find(tid);
		if (i != data->threadmap.end()) {
			// Have to detach before we erase; otherwise we trigger a terminate
			// in thread dtor, which recursively calls this method...
			i->second.associated_thread->detach();
			task_id_to_complete = i->second.associated_task->get_id();
			// This causes the thread object to be destroyed, among other
			// things.
			data->threadmap.erase(i);
		}
	} // release lock on threadmap
	if (task_id_to_complete != UINT64_MAX) {
		--data->active_thread_count;
		auto asgn = get_current_assignment();
		if (asgn) {
			bool all_complete = asgn->complete_task(task_id_to_complete);
			if (all_complete) {
				auto msg = serialize_msg(NITRO_1ASSIGNMENT_COMPLETE,
						asgn->get_id());
				queue_for_send(publisher, msg);
				lock_guard<mutex> lock(data->aqueue_mutex);
				data->asgn_queue.pop();
			}
		} else {
			xlog("Got thread exit without current assignment. Huh?");
		}
	} else {
		xlog("Didn't find thread in map.");
	}
}

worker_engine::worker_engine(cmdline const & cmdline) :
		engine(cmdline), data(new data_t) {

	auto subscriber = zmq_socket(ctx, ZMQ_SUB);
	if (subscriber) {

		data->subscriber = subscriber;
		zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, COORDINATION_TOPIC,
				strlen(COORDINATION_TOPIC));
		bind_after_ctor("w");

		auto wf = cmdline.get_option("--workfor", "");
		auto proto = strstr(wf, "://");
		if (proto) {
			wf = proto + 3;
		}
		if (*wf) {
			data->workfor = wf;
			auto wfport = strrchr(data->workfor.c_str(), ':');
			if (!wfport) {
				data->workfor += interp(":%1", DEFAULT_PUBSUB_PORT);
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
	return data->asgn_queue.empty() ? 0 : data->asgn_queue.front().get();
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
			assignment::tasklist_t const & readylist = asgn->get_list_by_status(
					task_status::ts_ready);
			std::list<task const *> to_start;
			for (auto i = readylist.cbegin(); i != readylist.cend(); ++i) {
				task const & t = **i;
				to_start.push_back(&t);
				if (++atc == desired_busy_threads) {
					break;
				}
			}
			if (!to_start.empty()) {
				lock_guard<mutex> lock(data->tmap_mutex);
				for (auto i : to_start) {
					auto cmdline = asgn->activate_task(i->get_id());
					thread * launched = data->launcher(*this, cmdline);
					thread_task_pair ttpair(launched, i);
					data->threadmap.insert( { launched->get_id(), ttpair });
				}
				data->active_thread_count += to_start.size();
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
	lock_guard<mutex> lock(data->aqueue_mutex);
	data->asgn_queue.push(assignment::handle(asgn));
	// Normally, we say we're enrolled when we receive a message requesting
	// help, and we affirm that we're available. But in testing, we may
	// directly call this method without sending a message. In all cases,
	// we must be enrolled by the time we accept an assignment, so this is a
	// useful failsafe.
	data->enrolled = true;
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
		queue_for_send(publisher, txt);
	} else {
		// TODO: REPORT IDLE
	}
}

int worker_engine::do_run() {

	const int ITEM_COUNT = 2;

	const auto REPORTING_INTERVAL = milliseconds(5000);
	high_resolution_clock clock;
	auto last_status_report = clock.now() - REPORTING_INTERVAL;

	while (true) {

		// Don't keep looping if we've completed our work.
		if (data->enrolled && !get_linger() && !get_current_assignment()) {
			break;
		}

		start_more_tasks();

		auto time_since_last_status_report = clock.now() - last_status_report;
		if (time_since_last_status_report > REPORTING_INTERVAL) {
			report_status();
			last_status_report = clock.now();
		}

		int delay = 25;
		while (true) {

			zmq_pollitem_t items[] = { { responder, 0, ZMQ_POLLIN, 0 }, {
					data->subscriber, 0, ZMQ_POLLIN, 0 }, };

			int rc = zmq_poll(items, ITEM_COUNT, delay);
			if (rc < 1) {
				break;
			} else {

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
									respond_to_help_request(socket));
						case NITRO_HERE_IS_ASSIGNMENT:
							IF_SOCKET_N_HANDLE(0,
									respond_to_assignment(socket, json));
						default:
							xlog("Unrecognized message %1 (%2)",
									events::get_std_id_repr(code),
									events::catalog().get_msg(code));
						}
					}
				}
				// On subsequent times through the loop, don't wait; only
				// continue looping as long as we have a backlog of messages.
				delay = 0;
			}
		}

		// Dispatch any messages that we've decided to send.
		send_queued();
	}

	return 0;
}

} // end namespace nitro
