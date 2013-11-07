#include <thread>

#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/engine.h"
#include "domain/worker_engine.h"

#include "gtest/gtest.h"

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

#define tryz(expr) rc = expr; if (rc) throw ERROR_EVENT(errno)

void coord_listener_thread_main(coord_engine const & coord,
		std::vector<std::string> & msgs) {

	try {
		coord.wait_until_ready();

	    void *context = coord.ctx; //zmq_ctx_new ();
	    void *subscriber = zmq_socket (context, ZMQ_SUB);
	    int rc;
	    tryz(zmq_connect (subscriber, "inproc://weather.ipc"));

	    rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE,
	                         "", 0);
	    assert (rc == 0);

	    //  Process 100 updates
	    int update_nbr;
	    long total_temp = 0;
	    for (update_nbr = 0; update_nbr < 100; update_nbr++) {
	    	char buf[20];
	        zmq_recv(subscriber, buf, 20, 0);

	        int zipcode, temperature, relhumidity;
	        sscanf (buf, "%d %d %d",
	            &zipcode, &temperature, &relhumidity);
	        fprintf(stderr, "%s\n", buf);
	        total_temp += temperature;
	    }

	    zmq_close (subscriber);
	    //zmq_ctx_destroy (context);
	} catch (std::exception const & e) {
		fprintf(stderr, e.what());
	}
}

TEST(engine_test, complete_batch_lifecycle) {
	auto coord = make_coord_engine();
	std::vector<std::string> msgs;
	// Have to do a little casting here. engine_handle is a handle to the base
	// class, but we need to pass a ref to the derived class...
	auto coord_ptr = reinterpret_cast<coord_engine *>(coord.get());
	std::thread listener(coord_listener_thread_main, std::ref(*coord_ptr),
			std::ref(msgs));
	coord->run();
}

