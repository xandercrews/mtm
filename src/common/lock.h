#ifndef _NITRO_COMMON_LOCK_H_
#define _NITRO_COMMON_LOCK_H_

#include <pthread.h>

namespace nitro {

/**
 * This global constant defines the default attributes that are recommended
 * for all mutexes. It's useful mainly in corner cases where you want to
 * init and lock a mutex yourself, before handing it to a Lock object.
 * In debug mode, it turns on many validating checks. It is deliberately NOT
 * recursive; see this rant about recursive mutexes from one of the designers
 * of pthreads: http://www.zaval.org/resources/library/butenhof1.html
 * Use this constant whenever you don't need funky, customized mutex attributes.
 */
extern pthread_mutexattr_t const * DEFAULT_MUTEX_ATTR;

/**
 * A Mutex is just a pthread_mutex_t that's automatically initialized and
 * cleaned up. Prefer using this class over a raw pthread_mutex_t whenever you
 * can, since it produces cleaner, less verbose, and more tested code.
 */
struct Mutex {
	/** Pass the address of this member to posix functions. */
	pthread_mutex_t internal;

	/** Init a Mutex, possibly with custom attributes. */
	Mutex(pthread_mutexattr_t const * attr=NULL);

	/**
	 * Allow a Mutex to be cast implicitly to a raw pthread_mutex_t so it can
	 * be passed to functions that expect the raw structure.
	 */
	operator pthread_mutex_t & () { return internal; }
	operator pthread_mutex_t const & () const { return internal; }
};

/**
 * A MutexAttr is just a pthread_mutexattr_t that's automatically initialized
 * and cleaned up. Prefer using this class over a raw pthread_mutexattr_t
 * whenever you can, since it produces cleaner, less verbos, and more tested
 * code.
 */
struct MutexAttr {
	/** Pass the address of this member to posix functions. */
	pthread_mutexattr_t internal;

	/** Init a MutexAttr by calling pthread_mutexattr_init(). */
	MutexAttr();

	/** Destroy a MutexAttr by calling pthread_mutexattr_destroy(). */
	~MutexAttr();

	/**
	 * Allow a MutexAttr to be cast implicitly to a raw pthread_mutexattr_t so
	 * it can be passed to functions that expect the raw structure.
	 */
	operator pthread_mutexattr_t & () { return internal; }
	operator pthread_mutexattr_t const & () const { return internal; }
};

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
	pthread_mutex_t & mutex;

	// Locks cannot be assigned or copy-constructed; we enforce that by making
	// these methods private. If you want to pass a Lock out of an inner scope,
	// use std::unique_ptr<Lock>.
	Lock & operator= (Lock const & rhs);
	Lock(Lock const & other);

	bool unlock();
	bool lock(unsigned timeout_nanosecs);

public:

	/**
	 * Create a lock on a mutex, or throw an exception after a timeout.
	 *
	 * @param mutex
	 *     This is a mutex owned by some other object. The lock does not
	 *     create it or destroy it; it just manages whether it's locked or
	 *     unlocked. Typically you'd create a static variable or instance
	 *     variable of type Mutex, and then pass it to this function. If the
	 *     mutex is an instance variable, you must guarantee that the owning
	 *     object's lifetime will exceed the lifetime of the Lock.
	 *
	 * @param timeout_millisecs
	 *     Abandon attempt to lock after the specified number of millisecs.
	 *     In order to prevent long-lasting deadlocks, there is no way to
	 *     specify "infinite" for this parameter.
	 * @pre
	 *     timeout_millis must be a value in the range [1..3600000).
	 *
	 * @param is_locked
	 *     If true, no new locking attempt is performed. This should be a rare
	 *     corner case that raises eyebrows, because it means we're doing an
	 *     end run around the locking mechanism somewhere. Try to avoid.
	 *
	 * @throws Error(NITRO_TIMED_OUT_AFTER_1MILLIS_LOCKING_MUTEX) if attempt to
	 *     lock timed out. This is a non-fatal error that can be trapped in a
	 *     loop if you want to retry.
	 *
	 * @throws Error(NITRO_FAILED_TO_LOCK_MUTEX_1POSIX_ERROR) or
	 *     Error(NITRO_DEADLOCK) if caller passed an unusable or already-locked
	 *     mutex. This is a fatal coding error, and it is should cause an
	 *     abend to prevent undefined behavior.
	 *
	 * The mutex is unlocked when the Lock goes out of scope, unless .detach()
	 * is called first.
	 */
	Lock(Mutex & mutex, unsigned timeout_millisecs = 5000,
			bool is_locked = false);

	/**
	 * Create a lock on a mutex, or throw an exception after a timeout. This
	 * override is provided for corner cases where a Mutex object can't be used
	 * and a raw pthread_mutex_t is necessary. It is NOT preferred; only use it
	 * when you must.
	 *
	 * @param raw_mutex
	 *     This is a mutex owned by some other object. The lock does not
	 *     create it or destroy it; it just manages whether it's locked or
	 *     unlocked. The same lifetime constraints apply as are described in
	 *     the simpler version of the constructor. The caller must have
	 *     called pthread_mutex_init() on raw_mutex before calling this
	 *     constructor.
	 *
	 * @param timeout_millisecs
	 *     Abandon attempt to lock after the specified number of millisecs.
	 *     In order to prevent long-lasting deadlocks, there is no way to
	 *     specify "infinite" for this parameter.
	 * @pre
	 *     timeout_millis must be a value in the range [1..3600000).
	 *
	 * @param is_locked
	 *     If true, no new locking attempt is performed. This should be a rare
	 *     corner case that raises eyebrows, because it means we're doing an
	 *     end run around the locking mechanism somewhere. Try to avoid.
	 *
	 * @throws Error(NITRO_TIMED_OUT_AFTER_1MILLIS_LOCKING_MUTEX) if attempt to
	 *     lock timed out. This is a non-fatal error that can be trapped in a
	 *     loop if you want to retry.
	 *
	 * @throws Error(NITRO_FAILED_TO_LOCK_MUTEX_1POSIX_ERROR) or
	 *     Error(NITRO_DEADLOCK) if caller passed an unusable or already-locked
	 *     mutex. This is a fatal coding error, and it is should cause an
	 *     abend to prevent undefined behavior.
	 *
	 * The mutex is unlocked when the Lock goes out of scope, unless .detach()
	 * is called first.
	 */
	Lock(pthread_mutex_t & mutex, unsigned timeout_millisecs = 5000,
			bool is_locked = false);

	~Lock();

	/**
	 * Tell the Lock that it should relinquish responsibility for managing the
	 * mutex that it owns. After this call, the Lock doesn't own anything. This
	 * allows the mutex to be transferred to other owners in the rare corner
	 * cases where that makes sense.
	 */
	pthread_mutex_t & detach();
};

} // end namespace nitro

#endif // sentry
