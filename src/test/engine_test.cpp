#include <thread>

#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/engine.h"
#include "domain/msg.h"
#include "domain/worker_engine.h"

#include "gtest/gtest.h"

#include "test/test_util.h"

#include "zeromq/include/zmq.hpp"

using namespace nitro;

engine::handle make_coord_engine(char const ** args = NULL, int argc = 0) {
	char const * def_args[] = {"nitro", "--rrport", "52500",
			"--psport", "52501", "--exechost", "localhost"};
	if (!args) {
		args = def_args;
		argc = 5;
	}
	cmdline cmdline(argc, args);
	if (!cmdline.get_errors().empty()) {
		for (auto e: cmdline.get_errors()) {
			ADD_FAILURE() << e.what();
		}
	}
	return make_engine(cmdline);
}

engine::handle make_worker_engine(char const ** args = NULL, int argc = 0) {
	char const * def_args[] = {"nitro", "--rrport", "52502",
			"--psport", "52503", "--workfor", "tcp://localhost:52500"};
	if (!args) {
		args = def_args;
		argc = 7;
	}
	cmdline cmdline(argc, args);
	if (!cmdline.get_errors().empty()) {
		for (auto e: cmdline.get_errors()) {
			ADD_FAILURE() << e.what();
		}
	}
	return make_engine(cmdline);
}

TEST(engine_test, coord_ctor_works) {
	auto engine = make_coord_engine();
	auto ptr = dynamic_cast<coord_engine *>(engine.get());
	ASSERT_TRUE(ptr != NULL);
}

TEST(engine_test, worker_ctor_works) {
	auto engine = make_worker_engine();
	auto ptr = dynamic_cast<worker_engine *>(engine.get());
	ASSERT_TRUE(ptr != NULL);
}

TEST(engine_test, coord_and_worker_can_coexist_on_same_box) {
	auto coord = make_coord_engine();
	ASSERT_TRUE(static_cast<bool>(coord));
	auto worker = make_worker_engine();
	ASSERT_TRUE(static_cast<bool>(worker));
}
