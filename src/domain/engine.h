#ifndef _NITRO_DOMAIN_ENGINE_H_
#define _NITRO_DOMAIN_ENGINE_H_

namespace nitro {

/**
 * Does most of the work of the app, and directly or indirectly manages the
 * lifetime of resources such as batches, threads, ports, etc.
 */
class Engine {

public:
	Engine();
	virtual ~Engine();

	// These properties can only be changed before the engine begins to run.
	// As a result, we don't need to synchronize them.

	void set_publish_port(int port);
	int get_publish_port() const;

	void set_listen_port(int port);
	int get_listen_port() const;

	void set_follow_mode(bool value);
	bool get_follow_mode() const;
/*
    - publish queue: zmq::socket_t
    - listen queue: zmq::socket_t
*/
	void handle_ping_request() const;
	void handle_terminate_request() const;
	void send_progress_report() const;

	int run();

private:
	int publish_port;
	int listen_port;
	bool follow_mode;
	bool running;
};

} /* namespace nitro */

#endif /* NITRO_DOMAIN_ENGINE_H_ */
