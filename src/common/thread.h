#ifndef _NITRO_COMMON_THREAD_H_
#define _NITRO_COMMON_THREAD_H_

#include <cstdint>
#include <stdexcept>
#include <pthread.h>

namespace nitro {

/**
 * Manage an independent thread of execution.
 *
 * This class wraps the posix threading APIs. The value of wrapping is that it
 * hides lots of messy details and makes sure that basic mechanics are done
 * in such a way that cleanup, error handling, and patterns of usage just work.
 *
 * This class is inherently thread-safe--meaning that you can call all of its
 * methods from any and all threads at the same time without race conditions,
 * deadlocks, or seg faults.
 */
class Thread {

public:
	/**
	 * The type of a unique identifier for a thread. These identifiers are
	 * guaranteed to be unique to the operating system, for all threads that
	 * are running. However, as soon as a thread exits, its ID can be recycled.
	 */
	typedef pthread_t ID;
	typedef void * (* StartFunc)(void *);

	/**
	 * Create a thread object that can be started with .start().
	 *
	 * By default, threads are joinable and run at normal priority. If you
	 * want to override this behavior, call the various set_XYZ() methods
	 * before you call .start().
	 */
	Thread();

	/**
	 * Destroy a thread and possibly kill the underlying posix thread in the
	 * process.
	 *
	 * If a joinable Thread object goes out of scope while it
	 * is still running, it is killed. If a detached Thread object goes out of
	 * scope, the underlying posix thread continues to run until main() exits.
	 * This may be dangerous, depending on whether the detached thread is
	 * holding references to state variables that are no longer valid. Use
	 * cautiously.
	 */
	virtual ~Thread();

	/**
	 * Start the thread using the specified function and arg.
	 *
	 * @pre startfunc must not be NULL.
	 */
	void start(StartFunc startfunc, void * arg = 0);

	/**
	 * Control whether the thread will run joinable or detached (which are
	 * mutually exclusive states).
	 *
	 * By default, threads are joinable. This provides a convenient synch
	 * mechanism, but it means that resources associated with the thread cannot
	 * be freed on thread exit; instead, they must be held until the thread
	 * has been joined or (in the worst case) until main() exits. Until that
	 * point, the thread is basically a zombie. If you know that you don't want
	 * to wait for the thread exit, detach it instead by calling
	 * set_joinable(false).
	 *
	 * @throws Error(NITRO_ILLEGAL_CALL_IN_STATE_1NAME) if this function is
	 *     called to make an alive and detached thread joinable, since no
	 *     adjustments to such a thread's state are possible.
	 */
	void set_joinable(bool value);

	/**
	 * @return true if the thread is in joinhas been put in detached mode by calling
	 *     set_detached(true) -- even if the thread has not yet started. Since
	 *     detached and joinable are mutually exclusive, a thread is joinable
	 *     when is_detached() returns false.
	 */
	bool is_joinable() const;

	/**
	 * Change the name of a thread. This can be useful in debugging. Names
	 * can be changed at any time.
	 *
	 * @param name
	 *     Case-sensitive. Must be < 16 chars long. Should not contain control
	 *     control chars or CR/LF.
	 *
	 * @pre name is valid.
	 */
	void set_name(char const * value);

	/**
	 * @return the name of the thread.
	 */
	char const * get_name() const;

	/**
	 * @return any exception thrown by the function that was invoked when a
	 *     Thread's .start() method was called. Before calling start(), and
	 *     after a clean exit, return NULL.
	 */
	std::exception * get_exception() const;

	/**
	 * @return any exit code produced by the function that was invoked when a
	 *     Thread's .start() method was called. Before calling start(), and
	 *     after a clean exit, return 0.
	 */
	int get_exit_code() const;

	/**
	 * @return true if thread is currently running; return false before
	 *     #start() is called, and after the thread terminates.
	 */
	bool is_alive() const;

	/**
	 * @return the ID of a thread. Before a thread starts, its id is 0. After
	 *     it starts, the ID is set to the id of the underlying posix thread.
	 *     After it terminates, the old ID is still returned, but it is no
	 *     longer valid to use with pthread calls.
	 */
	ID get_id() const;


	void interrupt();
	void join();

	/**
	 * More convenient than calling posix's nanosleep, because you don't have
	 * to fill a structure with the second portion and the leftover nanosecond
	 * portion.
	 *
	 * @throws Interrupt if a signal is received before the time elapses.
	 * @throws Error(posix_error) if something else goes wrong.
	 */
	static void nanosleep(uint64_t nanosecs);

	/**
	 * Sleep for a specified number of millisecs.
	 *
	 * @throws Interrupt if a signal is received before the time elapses.
	 * @throws Error(posix_error) if something else goes wrong.
	 */
	static void sleep(uint64_t millisecs);

	/**
	 * An exception raised when a thread gets interrupted.
	 */
	class Interrupt : public std::runtime_error {
		uint64_t remaining_time;
	public:
		Interrupt(uint64_t remaining_time = 0);
	};

private:
	struct Data;
	Data * data;

	// Disallow copy constructor and assignment operator.
	Thread(Thread const & rhs);
	Thread & operator =(Thread const & rhs);
};


} // end namespace

#endif /* THREAD_H_ */
