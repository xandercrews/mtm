#include <thread>
#include <random>
#include <sstream>

#include "base/countof.h"
#include "base/xlog.h"

#include "domain/assignment.h"
#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/worker_engine.h"

#include "gtest/gtest.h"

using std::thread;
using std::chrono::milliseconds;
using std::mt19937;
using std::stringstream;

using namespace nitro;

void fake_launch_thread_main(worker_engine * we, char const * cmdline) {
	worker_engine::notifier notifier(*we);
	mt19937 randomizer;
	xlog(cmdline);
	std::this_thread::sleep_for(milliseconds(randomizer() % 50));
}

thread fake_launch_func(worker_engine * we, char const * cmdline) {
	return thread(fake_launch_thread_main, we, cmdline);
}

TEST(worker_engine_test, assignment_mgmt) {

	char const * wargs[] = { "nitro", "--replyport", "36123", "--workfor",
			"127.0.0.1:36124" };

	worker_engine we(cmdline(countof(wargs), wargs));
	EXPECT_STREQ("127.0.0.1:36124", we.get_workfor());

	stringstream ss;
	mt19937 randomizer;
	for (int i = 0; i < 100; ++i) {
		auto cmd = interp("qsub -p %1{i} -l walltime=%2 %3\n", 1023 - i,
				(randomizer() % 30) * 10, randomizer());
		ss << cmd;
	}
	auto asgn = new assignment("a1", ss.str().c_str());
	we.accept_assignment(asgn);
	we.set_launch_func(fake_launch_func);
	we.run();
}
