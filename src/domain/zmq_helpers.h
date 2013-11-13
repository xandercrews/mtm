#ifndef _DOMAIN_ZMQ_HELPERS_H_
#define _DOMAIN_ZMQ_HELPERS_H_

#include "zeromq/include/zmq.h"

void zmq_close_now(void * socket);
int zmq_bind_and_log(void * socket, char const * endpoint);
int zmq_connect_and_log(void * socket, char const * endpoint);

struct zctx_cleaner {
	void * ctx;
	zctx_cleaner(void * ctx) : ctx(ctx) {}
	~zctx_cleaner() { if (ctx) zmq_ctx_destroy(ctx); }
};

struct zsocket_cleaner {
	void * socket;
	zsocket_cleaner(void * socket) : socket(socket) {}
	~zsocket_cleaner() { zmq_close_now(socket); }
};

#endif // sentry
