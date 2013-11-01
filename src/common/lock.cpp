#include "common/lock.h"
#include "common/dbc.h"

using namespace std;

namespace nitro {

Lock::Lock(PlatformMutex & mutex, unsigned timeout_millis, bool is_locked) :
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
	timeout.tv_nsec += 1000L * timeout_millis;
	int rc = pthread_mutex_timedlock(&mutex, &timeout);
	switch (rc) {
	case 0:
		break;
	case ETIMEDOUT:
		throw NITRO_ERROR(NITRO_TIMED_OUT_AFTER_1MILLIS_LOCKING_MUTEX,
				timeout_millis);
		break;
	default:
		throw NITRO_ERROR(NITRO_FAILED_TO_LOCK_MUTEX_1POSIX_ERROR, rc);
		break;
	}
	locked = true;
	return locked;
}

PlatformMutex & Lock::detach() {
	unlock_on_exit = false;
	return mutex;
}

} // end namespace nitro
