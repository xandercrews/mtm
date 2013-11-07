#include <condition_variable>
#include <thread>

#include "base/file_lines.h"
#include "base/strutil.h"

#include "domain/cmdline.h"
#include "domain/coord_engine.h"

#include "zeromq/include/zmq.hpp"

using namespace std;

namespace nitro {

coord_engine::coord_engine(cmdline const & cmdline) : engine(cmdline),
		ctx(_ctx), _ctx(0) {
	auto exec_host = cmdline.get_option("--exechost");
	if (!exec_host) {
		exec_host = getenv("exec_host");
	}
	if (exec_host) {
		try {
			file_lines fl(exec_host);
			while (true) {
				auto line = fl.next();
				if (line == nullptr) {
					break;
				}
#if 0
				string ln(line);
				split(ln, ',', hostlist);
#else
				hostlist.push_back(line);
#endif

			}
			return;
		} catch (error_event const & e) {
			// Must not be a file. Try raw list.
#if 0
			string eh(exec_host);
			split(trim(eh), ',', hostlist);
#else
			hostlist.push_back(exec_host);
#endif
		}
	}
}

void coord_engine::wait_until_ready() const {
	unique_lock<mutex> lock(ready_mutex);
	while (!_ctx) {
		ready_signal.wait(lock);
	}
}

coord_engine::~coord_engine() {
}

#define tryz(expr) rc = expr; if (rc) throw ERROR_EVENT(errno)

int coord_engine::run() {
    void *context = zmq_ctx_new ();
    void *publisher = zmq_socket (context, ZMQ_PUB);
    int rc = 0;//zmq_bind (publisher, "tcp://*:5556");
    assert (rc == 0);
    rc = zmq_bind (publisher, "inproc://weather.ipc");
    assert (rc == 0);

	// Tell any threads that are waiting for us to be ready to run, that we
	// are now good to go.
	{
		unique_lock<mutex> lock(ready_mutex);
		_ctx = context;
		ready_signal.notify_all();
	}
	// Give listening threads the opportunity to begin listening.
	this_thread::yield();

    while (1) {
        //  Get values that will fool the boss
        int zipcode = 3;
        int temperature = 4;
        int relhumidity = 5;

        //  Send message to all subscribers
        char update[20];
        sprintf(update, "%05d %d %d", zipcode, temperature, relhumidity);
        zmq_send(publisher, update, strlen(update) + 1, 0);
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    zmq_close (publisher);
    zmq_ctx_destroy (context);
    return 0;
#if 0
    try {
		int rc;
		tryz(zmq_bind(pub, "tcp://localhost:5555"));
		fprintf(stderr, "finished binding\n");
		while (true) {
			char buffer[10];
			tryz(zmq_send(pub, "hello", 6, 0));
			tryz(zmq_recv(pub, buffer, 10, 0));
		}
	} catch (runtime_error const & e) {
		fprintf(stderr, e.what());
	}
	zmq_close(pub);
	zmq_ctx_destroy(ctx);
	return 0;
#endif
}

} // end namespace nitro
