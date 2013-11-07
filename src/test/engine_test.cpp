/*
 * engine_test.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#include "domain/engine.h"
#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/worker_engine.h"

#include "gtest/gtest.h"

using namespace nitro;

engine_handle make_coord_engine(char const ** args = NULL, int argc = 0) {
	char const * def_args[] = {"progname", "--replyport", "52500",
			"--publishport", "52501", "--exechost", "localhost"};
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

engine_handle make_worker_engine(char const ** args = NULL, int argc = 0) {
	char const * def_args[] = {"progname", "--replyport", "52502",
			"--publishport", "52503", "--workfor", "tcp://localhost:52500"};
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

TEST(engine_test, manager_ctor_works) {
	auto engine = make_coord_engine();
	ASSERT_TRUE(dynamic_cast<coord_engine *>(engine.get()) != NULL);
}

TEST(engine_test, worker_ctor_works) {
	auto engine = make_worker_engine();
	ASSERT_TRUE(dynamic_cast<worker_engine *>(engine.get()) != NULL);
}

TEST(engine_test, manager_and_worker_can_coexist_on_same_box) {
	auto coord = make_coord_engine();
	ASSERT_TRUE(static_cast<bool>(coord));
	auto worker = make_worker_engine();
	ASSERT_TRUE(static_cast<bool>(worker));
}

TEST(engine_test, complete_batch_lifecycle) {
	auto coord = make_coord_engine();
	auto worker = make_worker_engine();

}

#if 0
TEST(engine_test, two_engines_cant_bind_same_ports) {
	auto e1 = make_coord_engine();
	ASSERT_TRUE(static_cast<bool>(e1));
	auto e2 = make_coord_engine();
	ASSERT_FALSE(static_cast<bool>(e2));
}
#endif
