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

engine::handle make_worker_engine(char const ** args = NULL, int argc = 0) {
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

#define tryz(expr) rc = expr; if (rc) throw ERROR_EVENT(errno)

void coord_listener_thread_main(coord_engine const & coord,
		std::vector<std::string> & ) {

	try {
	    void *context = coord.ctx;
	    void *subscriber = zmq_socket(context, ZMQ_SUB);
	    int rc;
	    tryz(zmq_connect(subscriber, coord.get_subscribe_endpoint("tcp")));
	    tryz(zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0));

	    for (int i = 0; i < 5; ++i) {
	    	auto txt = receive_full_msg(subscriber);
	        fprintf(stderr, "%s\n", txt.c_str());
	    }
	    zmq_close(subscriber);
	} catch (std::exception const & e) {
		fprintf(stderr, "%s\n", e.what());
	}
}
