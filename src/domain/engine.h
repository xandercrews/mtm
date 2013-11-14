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

const char * const IPC_PUB_BINDING = "ipc:///tmp/nitro-%1{pid}-%2{style}pub";
const char * const IPC_REP_BINDING = "ipc:///tmp/nitro-%1{pid}-%2{style}rep";

extern const unsigned MAX_HARDWARE_THREADS;

class cmdline;

/**
 * Engines subscribe to this topic to hear messages that they need to handle
 * to interoperate.
 */
extern const char * const COORDINATION_TOPIC;

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
	 * Identifies the endpoint to subscribe to if you want to listen to what
	 * this engine is doing as it runs.
	 */
	char const * get_subscribe_endpoint(char const * transport) const;

	/**
	 * What port are we listening on?
	 *
	 * We may also talk on this port, but we only do it in response to incoming
	 * requests; we don't proactively use this port to initiate new
	 * conversations. In this respect, we use the port in much the same way
	 * as a web server uses ports on which incoming http requests arrive.
	 */
	int get_reply_port() const;

	/**
	 * Should the engine wait for a terminate message before exiting?
	 */
	bool get_linger() const;

	/**
	 * What port do we use to initiate new conversations with others?
	 *
	 * In manager mode, we use this port to proactively communicate with our
	 * workers. In worker mode, we use this port for an occasional status
	 * report.
	 */
	int get_publish_port() const;

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
	int reply_port;
	int publish_port;
	void * _ctx;
	void * _responder;
	void * _publisher;
	std::string id;
	std::string ipc_pub_endpoint;
	std::string tcp_pub_endpoint;
	std::string ipc_rep_endpoint;
	std::string tcp_rep_endpoint;
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
