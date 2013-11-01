#ifndef _NITRO_COMMON_LOCK_H_
#define _NITRO_COMMON_LOCK_H_

#include <pthread.h>

namespace nitro {

/**
 * On platforms not using pthreads, an #ifdef'ed alternative for this data
 * type will keep the code clean.
 */
typedef pthread_mutex_t PlatformMutex;

/* Create an instance of this class when you want to lock a mutex; when the
 * destructor for the class fires, the mutex will automatically be unlocked,
 * even if gotos, exceptions, or premature returns are used.
 *
 * This class does not detect multiple attempts to lock the same resource on a
 * single thread. In other words, it is not recursive; put your lock outside
 * the recursion. That's way more efficient, anyway.
 *
 * Also, do not assume that locking everything is the right way to be thread-
 * safe. It's way more complicated than that. If you do not have confidence
 * about threading subtleties, talk to someone who does.
 */
class Lock {
	bool unlock_on_exit;
	bool locked;
	PlatformMutex & mutex;

	// Locks cannot be assigned or copy-constructed; we enforce that by making
	// these methods private. If you want to pass a Lock out of an inner scope,
	// use std::unique_ptr<Lock>.
	Lock & operator= (Lock const & rhs);
	Lock(Lock const & other);

	bool unlock();
	bool lock(unsigned timeout_millis);

public:

	/**
	 * Create a lock on a mutex.
	 *
	 * @param mutex
	 *     This is a mutex owned by some other object. The lock does not
	 *     create it or destroy it; it just manages whether it's locked or
	 *     unlocked. Typically you'd create a static variable or instance
	 *     variable of type PlatformMutex, and then pass it to this function.
	 *     If using instance variables, you must guarantee that the object's
	 *     lifetime will outlast the lifetime of the Lock.
	 *
	 * @param is_locked
	 *     If true, no new locking attempt is performed. This
	 *     should be a rare corner case that raises eyebrows, because it means
	 *     we're doing an end run around the locking mechanism somewhere. Try
	 *     to avoid.
	 *
	 * @param timeout_millis
	 *     Abandon attempt to lock after the specified number of millisecs.
	 *     In order to prevent long-lasting deadlocks, there is no way to
	 *     specify "infinite" for this parameter; it must be a value in the
	 *     range [1..3600000), or else Error(NITRO_PRECONDITION_1EXPR_VIOLATED)
	 *     is thrown.
	 *
	 * @throws Error(NITRO_TIMED_OUT_AFTER_1MILLIS_LOCKING_MUTEX) if attempt to
	 *     lock timed out. This is a non-fatal error that can be trapped in a
	 *     loop if you want to retry.
	 *
	 * @throws Error(NITRO_FAILED_TO_LOCK_MUTEX_1POSIX_ERROR) if caller passed
	 *     an unusable or already-locked mutex. This is a fatal coding error,
	 *     and it is should cause an abend to prevent undefined behavior.
	 *
	 * The mutex is released when the Lock goes out of scope, unless .detach()
	 * is called first.
	 */
	Lock(PlatformMutex & mutex, unsigned timeout_millis=5000,
			bool is_locked = false);
	~Lock();

	/**
	 * Tell the Lock that it should relinquish responsibility for managing the
	 * mutex that it owns. After this call, the Lock doesn't own anything. This
	 * allows the mutex to be transferred to other owners in the rare corner
	 * cases where that makes sense.
	 */
	PlatformMutex & detach();
};

} // end namespace nitro

#endif // sentry
