#ifndef _NITRO_DOMAIN_ENGINE_H_
#define _NITRO_DOMAIN_ENGINE_H_

namespace nitro {

/**
 * Does most of the work of the app, and directly or indirectly manages the
 * lifetime of resources such as batches, threads, ports, etc.
 */
class engine {

public:
	engine();
	virtual ~engine();

	void set_publish_port(int port);
	int get_publish_port() const;

	void set_listen_port(int port);
	int get_listen_port() const;

	void set_follow_mode(bool value);
	bool get_follow_mode() const;

	char const * get_transport() const;
	void set_transport(char const * value);

	void handle_ping_request() const;
	void handle_terminate_request() const;
	void send_progress_report() const;

	int run();

private:
	void init_zmq();

	struct data_t;
	data_t * data;
};

} // end namespace nitro

#endif // sentry
