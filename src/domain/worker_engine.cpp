#include "domain/worker_engine.h"

namespace nitro {

worker_engine::worker_engine(int reply_port, int publish_port) :
		engine(reply_port, publish_port) {
}

worker_engine::~worker_engine() {
}

} /* namespace nitro */
