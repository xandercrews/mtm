#include "domain/worker_engine.h"

namespace nitro {

worker_engine::worker_engine(cmdline const & cmdline) :
		engine(cmdline) {
}

worker_engine::~worker_engine() {
}

int worker_engine::do_run() {
	return 0;
}

} /* namespace nitro */
