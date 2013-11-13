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

#include "json/json.h"

#include "zeromq/include/zmq.h"

using namespace std;
using namespace nitro::event_codes;

namespace nitro {

typedef std::list<thread> threadlist_t;

struct worker_engine::data_t {
	void * subscriber;
	threadlist_t threadlist;
};

worker_engine::worker_engine(cmdline const & cmdline) :
		engine(cmdline), data(new data_t) {

	auto subscriber = zmq_socket(ctx, ZMQ_SUB);
	data->subscriber = subscriber;
	if (subscriber) {
		string eth = cmdline.get_option("--ethernet",
				DEFAULT_ETHERNET_INTERFACE);
		string endpoint = "epgm://" + eth + ";239.192.1.1:5555";

		zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, _subscription.c_str(),
				_subscription.size());

		int rc = zmq_connect(subscriber, endpoint.c_str());
		xlog("connect to pgm: %1", rc);
		rc = zmq_connect(subscriber, DEFAULT_IPC_ENDPOINT);
		xlog("connect to ipc: %1", rc);
	}
}

worker_engine::~worker_engine() {
	int linger = 0;
	if (data->subscriber) {
		zmq_setsockopt(data->subscriber, ZMQ_LINGER, &linger, sizeof(linger));
		zmq_close(data->subscriber);
	}
	delete data;
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

} /* namespace nitro */
