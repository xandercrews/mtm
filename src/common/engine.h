#ifndef _NITRO_COMMON_ENGINE_H_
#define _NITRO_COMMON_ENGINE_H_

namespace nitro {

/**
 * Does most of the work of the app, and directly or indirectly manages the
 * lifetime of resources such as batches, threads, ports, etc.
 */
class Engine {

public:
	Engine();
	virtual ~Engine();

	void set_port(int port);
	void set_slave_mode(bool value);
	int run();
};

} /* namespace nitro */

#endif /* NITRO_COMMON_ENGINE_H_ */
