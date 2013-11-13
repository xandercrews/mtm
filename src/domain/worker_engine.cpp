#include <condition_variable>
#include <thread>
#include <mutex>
#include <list>
#include <string.h>

#include "base/dbc.h"
#include "base/file_lines.h"
#include "base/strutil.h"
#include "base/xlog.h"

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

using namespace nitro::event_codes;

namespace nitro {

void default_thread_main(char const * cmdline) {
	FILE * f = popen(cmdline, "r");
	if (f) {
		char buf[1024];
		while (fgets(buf, sizeof(buf), f) != NULL) {
			;
		}
		pclose(f);
	}
}

thread default_launch_func(char const * cmdline) {
	return thread(default_thread_main, cmdline);
}

typedef std::list<thread> threadlist_t;

struct worker_engine::data_t {
	void * subscriber;
	threadlist_t threadlist;
	string workfor;
	launch_func launcher;

	data_t() :
			subscriber(0), threadlist() {
	}
};


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

int worker_engine::do_run() {

	if (data->subscriber) {
		Json::Value root;

		// Waits for request from the server
		xlog("waiting for coordinator...");
		string json = receive_full_msg(data->subscriber);

		if (json.size() && deserialize_msg(json, root)) {
			switch (root["body"]["code"].asInt()) {
			case NITRO_REQUEST_HELP:
				xlog("got request help command");
				// TODO: send response
				break;

			default:
				xlog("got unknown command");
				break;
			}
		}
	}
	return 0;
}

} // end namespace nitro
