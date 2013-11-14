#include <atomic>
#include <map>
#include <random>
#include <sstream>
#include <thread>

#include "base/countof.h"
#include "base/xlog.h"

#include "domain/assignment.h"
#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/event_codes.h"
#include "domain/msg.h"
#include "domain/worker_engine.h"
#include "domain/zmq_helpers.h"

#include "gtest/gtest.h"

#include "zeromq/include/zmq.h"

using std::thread;
using std::chrono::milliseconds;
using std::mt19937;
using std::stringstream;
using std::atomic;
using std::string;
using std::chrono::high_resolution_clock;
using std::map;

using namespace nitro;
using namespace nitro::event_codes;

atomic<int> thread_launch_count(0);
atomic<int> thread_exit_count(0);

void fake_launch_thread_main(worker_engine & we, char const * cmdline) {
	worker_engine::notifier notifier(we);
	struct exit_counter {
		~exit_counter() { ++thread_exit_count; }
	} ec;
	++thread_launch_count;
	mt19937 randomizer;
	xlog(cmdline);
	std::this_thread::sleep_for(milliseconds(randomizer() % 50));
}

thread * fake_launch_func(worker_engine & we, char const * cmdline) {
	return new thread(fake_launch_thread_main, std::ref(we), cmdline);
}

void listener_thread_main(worker_engine & we, map<int, int> & msg_counts) {

	high_resolution_clock clock;
	auto start = clock.now();

	void * subscriber = zmq_socket(we.ctx, ZMQ_SUB);
	zsocket_cleaner zclean(subscriber);
	int rc = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);
	if (rc) {
		xlog(ERROR_EVENT(errno).what());
	}
	auto endpoint = we.get_subscribe_endpoint("ipc");
	zmq_connect_and_log(subscriber, endpoint);

	while (true) {
		zmq_pollitem_t items[] = {
			{ subscriber, 0, ZMQ_POLLIN, 0},
		};
		int rc = zmq_poll(items, 1, 25);
		if (rc == 1) {
			auto txt = receive_full_msg(items[0].socket);
			if (!txt.empty()) {
				auto p = strstr(txt.c_str(), "0x");
				if (p) {
					char * end;
					auto n = strtol(p+2, &end, 16);
					auto x = msg_counts.find(n);
					if (x == msg_counts.end()) {
						msg_counts[n] = 1;
					} else {
						msg_counts[n]++;
					}
					if (n == NITRO_1ASSIGNMENT_COMPLETE) {
						return;
					}
				}
			}
		}
	}
}

TEST(worker_engine_test, assignment_mgmt) {

	char const * wargs[] = { "nitro", "--replyport", "36123", "--workfor",
			"127.0.0.1:36124" };

	worker_engine we(cmdline(countof(wargs), wargs));
	EXPECT_STREQ("127.0.0.1:36124", we.get_workfor());

	// Create a bunch of semi-realistic qsub command lines.
	const size_t TASK_COUNT = 100;
	stringstream ss;
	mt19937 randomizer;
	for (size_t i = 0; i < TASK_COUNT; ++i) {
		auto cmd = interp("qsub -p %1{i} -l walltime=%2 %3\n", 1023 - i,
				(randomizer() % 30) * 10, randomizer());
		ss << cmd;
	}

	// Make an assignment and give it to the worker_engine directly (no zmq
	// involved in this test).
	auto asgn = new assignment("a1", ss.str().c_str());
	we.accept_assignment(asgn);

	// Make sure that all threads launched by the worker engine do fake work
	// instead of shelling to real binaries.
	we.set_launch_func(fake_launch_func);

	// Launch listener thread so we can notice what happens while engine runs.
	map<int, int> counts;
	thread listener(listener_thread_main, std::ref(we), std::ref(counts));

	// Run engine. Prove that all tasks were launched.
	thread_launch_count.store(0);
	we.run();
	EXPECT_EQ(TASK_COUNT, thread_exit_count.load());
	EXPECT_EQ(TASK_COUNT, thread_launch_count.load());

	listener.join();

	xlog("should be finishing now");
}
