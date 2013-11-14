#ifndef _DOMAIN_WORKER_ENGINE_H_
#define _DOMAIN_WORKER_ENGINE_H_

#include <thread>

#include "domain/engine.h"

namespace Json {
	class Value;
}

namespace nitro {

class assignment;

/**
 * The engine used when the app is in "worker" mode, waiting for instructions
 * from a coordinator.
 */
class worker_engine : public engine {

public:
	worker_engine(cmdline const & cmdline);
	virtual ~worker_engine();

	/**
	 * An engine handle manages the lifetime of an engine instance. When the
	 * handle goes out of scope, the engine automatically shuts down (cleanly),
	 * and all resources are released. It goes dark on the network.
	 *
	 * handle objects can be passed from one owner to another, like batons.
	 */
	typedef std::unique_ptr<engine> handle;

	/**
	 * Return who I'm working for.
	 */
	const char * get_workfor() const;

	typedef std::thread * (*launch_func)(worker_engine &, char const * cmdline);
	launch_func get_launch_func() const;
	/**
	 * Allow launch behaviors to be simulated.
	 */
	void set_launch_func(launch_func value);

	/**
	 * All threads that this engine starts need to call this method on exit.
	 * Use the notifier class to guarantee this.
	 */
	void notify_thread_complete(std::thread::id id);
	struct notifier {
		worker_engine & we;
		notifier(worker_engine & we) : we(we) {}
		~notifier();
	};

	/**
	 * Should not be called by ordinary users. Public for testing.
	 */
	void accept_assignment(assignment * asgn);

protected:
	virtual int do_run();

private:
	struct data_t;
	data_t * data;

	void report_status();
	void respond_to_help_request(void * socket);
	void respond_to_assignment(void * socket, Json::Value const & json);
	void start_more_tasks();
	assignment * get_current_assignment() const;

	friend void zmq_poll_thread_main(worker_engine *);
};

} // end namespace nitro

#endif // sentry
