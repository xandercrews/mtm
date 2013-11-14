#include <thread>

#include "base/countof.h"
#include "base/xlog.h"

#include "domain/assignment.h"
#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/event_codes.h"
#include "domain/msg.h"
#include "domain/zmq_helpers.h"

#include "gtest/gtest.h"

#include "zeromq/include/zmq.h"

using std::thread;
using std::string;

using namespace nitro;
using namespace nitro::event_codes;

void driver_thread_main(coord_engine const & ce) {
	void * requester = zmq_socket(ce.ctx, ZMQ_REQ);
	zsocket_cleaner zclean(requester);
	auto endpoint = ce.get_endpoint(ep_reqrep, et_inproc);
	zmq_connect_and_log(requester, endpoint);
}

#if 0
TEST(coord_engine_test, req_rep) {
	char const * cargs[] = { "nitro", "--rrport", "36123", "--workfor",
			"127.0.0.1:36124" };

	coord_engine ce(cmdline(countof(cargs), cargs));
	ce.run();
}
#endif
