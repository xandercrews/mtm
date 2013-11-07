#include "zmq.hpp"
#include "gtest/gtest.h"

/**
 * This set of tests is mostly for us to understand how zmq works, not to
 * validate that it works correctly. It documents our assumptions.
 */

TEST(zmq_test, use_sockets_then_close_ctx_doesnt_hang) {
	{
		zmq::context_t ctx;
		{
			zmq::socket_t socket(ctx, ZMQ_PUB);
			socket.connect("tcp://localhost:56231");
			// when socket's destructor fires, it should set linger to 0, which
			// should cause immediate tear-down of socket on the box.
		}
		// When ctx goes out of scope, we should call zmq_ctx_destroy(), and
		// it should not hang, because our sockets should be properly torn
		// down.
	}
}
