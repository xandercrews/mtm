#include "domain/coord_engine.h"

namespace nitro {

coord_engine::coord_engine(int reply_port, int publish_port) :
		engine(reply_port, publish_port) {
}

coord_engine::~coord_engine() {
}

} // end namespace nitro
