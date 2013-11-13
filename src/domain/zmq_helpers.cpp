#include "base/error.h"
#include "base/xlog.h"

#include "zeromq/include/zmq.h"

#define ERR_IF_APPLICABLE rc ? ERROR_EVENT(errno).what() : "0 (success)"

void zmq_close_now(void * socket) {
	if (socket) {
		int linger = 0;
		int rc = zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));
		if (rc != 0) {
			xlog("Attempt to set ZMQ_LINGER to 0 returned %1.",
					ERROR_EVENT(errno).what());
		}
		zmq_close(socket);
	}
}

int zmq_bind_and_log(void * socket, char const * endpoint) {
	auto rc = zmq_bind(socket, endpoint);
	xlog("bind %1 returned %2", endpoint, ERR_IF_APPLICABLE);
	return rc;
}

int zmq_connect_and_log(void * socket, char const * endpoint) {
	auto rc = zmq_connect(socket, endpoint);
	xlog("connect %1 returned %2", endpoint, ERR_IF_APPLICABLE);
	return rc;
}
