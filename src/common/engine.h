/*
 * engine.h
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#ifndef ENGINE_H_
#define ENGINE_H_

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

#endif /* ENGINE_H_ */
