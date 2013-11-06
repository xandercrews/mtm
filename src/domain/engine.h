#ifndef _NITRO_DOMAIN_ENGINE_H_
#define _NITRO_DOMAIN_ENGINE_H_

#include <memory>
#include <string>

namespace nitro {

class cmdline;

/**
 * An interface for the workhorse of our application. More than one type of
 * engine is available to us, but clients only get to interact with the engine
 * through this public and fairly abstract interface.
 *
 * The ctor for this class is protected by design. The only way to create
 * engines is to use the abstract factory.
 */
class engine {
public:
	virtual ~engine();

	int run();

	int get_server_port() const;
	int get_client_port() const;
	std::string const & get_transport() const;
	virtual bool is_follower() const = 0;
	void handle_ping_request(/*zmq::message_t const & msg*/) const;
	void handle_terminate_request(/*zmq::message_t const & msg*/) const;

protected:
	engine(int server_port, int client_port, char const * transport);

private:
	int server_port;
	int client_port;
	std::string transport;
	void * zmq_ctx;
	void * zmq_server_socket;
	void * zmq_client_socket;
};

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

	engine_handle make(cmdline const & cmdline, char const * tag = NULL) const;

	/**
	 * Different classes register with the factory so it knows how to make
	 * them. Clients don't need to worry about this.
	 */
	typedef engine_handle (*engine_ctor)(int server_port, int client_port,
			char const * transport);
	bool register_ctor(engine_ctor ctor, bool follow_mode,
			char const * tag);

	engine_factory();
	~engine_factory();
private:
	struct data_t;
	data_t * data;
};

} // end namespace nitro

#endif // sentry
