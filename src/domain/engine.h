#ifndef _NITRO_DOMAIN_ENGINE_H_
#define _NITRO_DOMAIN_ENGINE_H_

#include <memory>
#include <string>

namespace nitro {

const char * const INPROC_PASSIVE_BINDING = "inproc://passive%1{engine_id}";
const char * const INPROC_ACTIVE_BINDING = "inproc://active%1{engine_id}";

class cmdline;

/**
 * An interface for the workhorse of our application. More than one type of
 * engine is available to us, but clients only get to interact with the engine
 * through this public and fairly abstract interface.
 *
 * The ctor for this class is protected by design. The only way to create
 * engines is to use the abstract factory (see below).
 */
class engine {
public:
	virtual ~engine();

	int run();

	/**
	 * Get a guid-like string that distinguishes this engine instance from all
	 * others. This guid will be different each time an engine object is
	 * created; it is not the same across runs of the program.
	 */
	char const * get_id() const;

	/**
	 * What port are we listening on?
	 *
	 * We may also talk on this port, but we only do it in response to incoming
	 * requests; we don't proactively use this port to initiate new
	 * conversations. In this respect, we use the port in much the same way
	 * as a web server uses ports on which incoming http requests arrive.
	 */
	int get_passive_port() const;

	/**
	 * What port do we use to initiate new conversations with others?
	 *
	 * In leader mode, we use this port to proactively communicate with our
	 * followers. In follower mode, we use this port for an occasional status
	 * report.
	 */
	int get_active_port() const;

	virtual bool is_follower() const = 0;
	void handle_ping_request(/*zmq::message_t const & msg*/) const;
	void handle_terminate_request(/*zmq::message_t const & msg*/) const;

protected:
	engine(int passive_port, int active_port);

private:
	int passive_port;
	int active_port;
	void * zmq_ctx;
	void * zmq_passive_socket;
	void * zmq_active_socket;
	std::string id;
};

/**
 * An engine_handle manages the lifetime of an engine instance. When the
 * handle goes out of scope, the engine automatically shuts down (cleanly),
 * and all resources are released. It goes dark on the network.
 *
 * engine_handle objects can be passed from one owner to another, like batons.
 */
typedef std::unique_ptr<engine> engine_handle;

/**
 * An abstract factory that will deliver an engine to be our workhorse. Typical
 * usage:
 *
 *     engine_factory & factory = engine_factory::singleton();
 *     engine_handle my_engine = factory.make(cmdline);
 */
class engine_factory {
public:
	static engine_factory & singleton();

	/**
	 * Create a new instance of an engine that's appropriate for the cmdline
	 * we've been given.
	 *
	 * @tag If present, require an engine that supports the specified feature.
	 *     This is for advanced use cases such as simulation, and is unnecessary
	 *     in normal usage.
	 */
	engine_handle make(cmdline const & cmdline, char const * tag = NULL) const;

	/**
	 * Different classes register with the factory so it knows how to make
	 * them. Clients don't need to worry about this.
	 */
	typedef engine_handle (*engine_ctor)(int passive_port, int active_port);
	bool register_ctor(engine_ctor ctor, bool follow_mode, char const * tag);

	engine_factory();
	~engine_factory();
private:
	struct data_t;
	data_t * data;
};

} // end namespace nitro

#endif // sentry
