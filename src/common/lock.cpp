#include "common/lock.h"
#include "common/dbc.h"

using namespace std;

namespace nitro {

// We do NOT want a recursive mutex. Using recursive mutexes is usually a code
// smell. See http://www.zaval.org/resources/library/butenhof1.html, which is
// a rant about recursive mutexes from one of the designers of pthreads.
const int MUTEX_TYPE =
#ifdef _DEBUG
		PTHREAD_MUTEX_ERRORCHECK;
#else
		PTHREAD_MUTEX_NORMAL;
#endif

MutexAttr::MutexAttr() {
	pthread_mutexattr_init(&internal);
}

MutexAttr::~MutexAttr() {
	pthread_mutexattr_destroy(&internal);
}

/**
 * By wrapping this global constant in a getter function, we avoid order-of
 * initialization problems. By using a MutexAttr instead of a raw
 * pthread_mutexattr_t, we guarantee that pthread_mutexattr_destroy will be
 * called when main() exits.
 */
pthread_mutexattr_t const * _get_default_mutex_attr() {
	static MutexAttr attr;
	static bool inited = false;
	if (!inited) {
		inited = true;
		pthread_mutexattr_settype(&attr.internal, MUTEX_TYPE);
	}
	return &attr.internal;
}

pthread_mutexattr_t const * DEFAULT_MUTEX_ATTR = _get_default_mutex_attr();

Mutex::Mutex(pthread_mutexattr_t const * attr) {
	if (attr == NULL) {
		attr = DEFAULT_MUTEX_ATTR;
	}
	pthread_mutex_init(&internal, attr);
}

Lock::Lock(Mutex & mutex, unsigned timeout_millis, bool is_locked) :
		mutex(mutex.internal), locked(is_locked), unlock_on_exit(true) {

	if (is_locked == false) {
		lock(timeout_millis);
	}
}

Lock::Lock(pthread_mutex_t & mutex, unsigned timeout_millis, bool is_locked) :
		mutex(mutex), locked(is_locked), unlock_on_exit(true) {

	if (is_locked == false) {
		lock(timeout_millis);
	}
}

Lock::~Lock() {
	if (locked && unlock_on_exit) {
		unlock();
	}
}

bool Lock::unlock() {
	if (!locked) {
		return false;
	}

	int rc = pthread_mutex_unlock(&mutex);
	if (rc != 0) {
		throw NITRO_ERROR(NITRO_FAILED_TO_UNLOCK_MUTEX_1POSIX_ERROR, rc);
	}
	locked = false;
	return true;
}

bool Lock::lock(unsigned timeout_millis) {
	if (locked) {
		return false;
	}

	PRECONDITION(timeout_millis > 0 && timeout_millis < 3600000);

	timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_nsec += 1000000L * timeout_millis;
	//int rc = pthread_mutex_lock(&mutex);
	int rc = pthread_mutex_timedlock(&mutex, &timeout);
	switch (rc) {
	case 0:
		break;
	case EDEADLK:
	case EINVAL:
		throw NITRO_ERROR(NITRO_DEADLOCK);
	case ETIMEDOUT:
		throw NITRO_ERROR(NITRO_TIMED_OUT_AFTER_1MILLIS_LOCKING_MUTEX,
				timeout_millis);
	default:
		throw NITRO_ERROR(NITRO_FAILED_TO_LOCK_MUTEX_1POSIX_ERROR, rc);
	}
	locked = true;
	return locked;
}

pthread_mutex_t & Lock::detach() {
	unlock_on_exit = false;
	return mutex;
}

} // end namespace nitro
