#ifndef MTM_COMMON_ENGINE_H_
#define MTM_COMMON_ENGINE_H_

namespace mtm {

class Engine {
public:
	Engine();
	virtual ~Engine();

	void set_port(int port);
	void set_slave_mode(bool value);
	int run();
};

} /* namespace mtm */

#endif /* MTM_COMMON_ENGINE_H_ */
