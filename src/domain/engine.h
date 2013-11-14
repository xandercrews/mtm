#ifndef _NITRO_DOMAIN_ENGINE_H_
#define _NITRO_DOMAIN_ENGINE_H_

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <utility>

namespace zmq {
	class context_t;
}

namespace nitro {

const char * const IPC_PUBSUB_ENDPOINT_PATTERN = "ipc:///tmp/nitro-%1{pid}-%2{style}pubsub";
const char * const IPC_REQREP_ENDPOINT_PATTERN = "ipc:///tmp/nitro-%1{pid}-%2{style}reqrep";
const char * const INPROC_PUBSUB_ENDPOINT_PATTERN = "inproc://%1{style}pubsub";
const char * const INPROC_REQREP_ENDPOINT_PATTERN = "inproc://%1{style}reqrep";
const char * const TCP_BIND_ENDPOINT_PATTERN = "tcp://*:%1";
const char * const TCP_CONNECT_ENDPOINT_PATTERN = "tcp://%1:%2";

extern const unsigned MAX_HARDWARE_THREADS;

class cmdline;
class assignment;

/**
 * Engines subscribe to this topic to hear messages that they need to handle
 * to interoperate.
 */
extern const char * const COORDINATION_TOPIC;

/**
 * Describe which transport we're working with.
 */
enum endpoint_transport {
	et_tcp,
	et_ipc,
	et_inproc
};

/**
 * Describe how we're using an endpoint.
 */
enum endpoint_pattern {
	ep_pubsub,
	ep_reqrep,
};

/**
 * A base class for the workhorses of our application.
 */
class engine {
public:
	engine(cmdline const &);
	virtual ~engine();

	/**
	 * Do all work of the engine, and return a posix-style exit code suitable
	 * for returning from main.
	 */
	int run();

	/**
	 * Get a guid-like string that distinguishes this engine instance from all
	 * others. This guid will be different each time an engine object is
	 * created; it is not the same across runs of the program.
	 */
	char const * get_id() const;

	/**
	 * Name the endpoint to use for interacting with this engine with the
	 * specified pattern and transport.
	 */
	char const * get_endpoint(int ep_pattern, int et_transport) const;

	/**
	 * What port are we using for a particular pattern?
	 */
	int get_port(int ep_pattern) const;

	/**
	 * Should the engine wait for a terminate message before exiting?
	 */
	bool get_linger() const;

	void handle_ping_request(/*zmq::message_t const & msg*/) const;
	void handle_terminate_request(/*zmq::message_t const & msg*/) const;

	/**
	 * An engine handle manages the lifetime of an engine instance. When the
	 * handle goes out of scope, the engine automatically shuts down (cleanly),
	 * and all resources are released. It goes dark on the network.
	 *
	 * handle objects can be passed from one owner to another, like batons.
	 */
	typedef std::unique_ptr<engine> handle;

	// Allow other threads to see our zeromq context for coordination
	// purposes. This is fully thread-safe.
	//
	// Normally, we'd expose internals only through const getters; exposing
	// internal members on a class that has to enforce the integrity of its
	// state is a VERY bad idea. However, a zeromq context is intended to be
	// shared across all threads in a process, and by making it a reference,
	// we make it impossible for anyone to change its value, so we're safe.
	void * const & ctx;

protected:
	virtual int do_run() = 0;

	// Allow derived classes to use the sockets we've opened, without
	// reassigning them.
	void * const & responder;
	void * const & publisher;

	// Derived classes should call this at the end of their constructor. We
	// can't bind completely until we know what style of engine we are.
	void bind_after_ctor(char const * style);
	void queue_for_send(void * socket, std::string const & msg);
	void send_queued();

private:
	int ports[2];

	void * _ctx;
	void * sockets[2];
	std::string id;
	std::string endpoints[2][3];

	typedef std::pair<void *, std::string> qmsg_t;
	std::queue<qmsg_t> send_queue;
	std::mutex send_queue_mutex;
	bool linger;
};

/**
 * Factory method. Choose which type of engine to make based on content of
 * cmdline; init from cmdline choices.
 */
engine::handle make_engine(cmdline const & cmdline);

} // end namespace nitro

#endif // sentry
