#include <condition_variable>
#include <thread>

#include "base/file_lines.h"
#include "base/strutil.h"

#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/msg.h"

#include "zeromq/include/zmq.h"

using namespace std;

namespace nitro {

coord_engine::coord_engine(cmdline const & cmdline) : engine(cmdline) {

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

coord_engine::~coord_engine() {
}

#define tryz(expr) rc = expr; if (rc) throw ERROR_EVENT(errno)

int coord_engine::do_run() {
    for (int i = 0; i < 7; ++i) {
        send_full_msg(publisher, "hello");
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    return 0;
}

} // end namespace nitro
