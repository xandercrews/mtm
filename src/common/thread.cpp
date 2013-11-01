#include <time.h>
#include <string.h>
#include <signal.h>

#include "common/thread.h"
#include "common/error.h"
#include "common/lock.h"
#include "common/dbc.h"

namespace nitro {

/**
 * Detects thread life, but doesn't enforce locking. Called by various
 * routines that *are* locked.
 */
inline bool _thread_is_alive(Thread::ID tid) {
	if (tid != 0) {
		// Calling pthread_kill() with sig==0 sends no signal, but will return
		// ESRCH if the id is no longer valid.
		int rc = pthread_kill(tid, 0);
		return (rc == 0);
	}
	return false;
}

struct Thread::Data {
	ID id;
	StartFunc startfunc;
	void * arg;
	int exit_code;
	Mutex mutex;
	std::exception * exception;
	bool joinable;
	std::string name;

	Data() : id(0), startfunc(0), arg(0), exit_code(0), exception(0),
			joinable(true) {
	}

	~Data() {
		// Unlike malloc/free-style code, deleting a null pointer is perfectly
		// legal and safe. So we don't have to test for null here.
		delete exception;
	}
};

Thread::Thread(): data(new Data) {
}

Thread::~Thread() {
	if (is_alive()) {
		interrupt();
	}
	delete data;
}

Thread::ID Thread::get_id() const {
	Lock lock(data->mutex);
	return data->id;
}

char const * Thread::get_name() const {
	Lock lock(data->mutex);
	return data->name.c_str();
}

void Thread::set_joinable(bool value) {
	Lock lock(data->mutex);
	// Are we changing anything? It's worth asking because trying to alter
	// the mode of an already-detached thread is impossible.
	if (data->joinable != value) {
		if (_thread_is_alive(data->id)) {
			if (!data->joinable) {
				throw NITRO_ERROR(NITRO_ILLEGAL_CALL_IN_STATE_1NAME, "detached");
			}
			data->joinable = value;
		}
	}
}

void Thread::set_name(char const * name) {
	if (name == NULL) {
		name = "";
	}
	size_t len = strlen(name);
	bool name_is_valid = len < 16;
	if (name_is_valid) {
		char const * end = name + len;
		for (char const * p = name; p < end; ++p) {
			if (*p < 32) {
				name_is_valid = false;
				break;
			}
		}
	}
	PRECONDITION(name_is_valid);
	Lock lock(data->mutex);
	data->name = name;
	if (_thread_is_alive(data->id)) {
		pthread_setname_np(data->id, name);
	}
}

bool Thread::is_alive() const {
	Lock lock(data->mutex);
	return _thread_is_alive(data->id);
}

std::exception * Thread::get_exception() const {
	Lock lock(data->mutex);
	return data->exception;
}

int Thread::get_exit_code() const {
	Lock lock(data->mutex);
	return data->exit_code;
}

void Thread::start(StartFunc startfunc, void * arg) {
	PRECONDITION(startfunc != 0);
	Lock lock(data->mutex);
	if (data->id == 0) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		// TODO: support customized posix thread attributes such as different
		// priorities, different scheduling policies, detached, etc.
		pthread_create(&data->id, &attr, startfunc, arg);
		if (!data->name.empty()) {
			pthread_setname_np(data->id, data->name.c_str());
		}
		pthread_attr_destroy(&attr);
	}
}

const int BILLION = 1000 * 1000 * 1000;

void Thread::nanosleep(uint64_t nanosecs) {

	timespec requested;
	memset(&requested, 0, sizeof(timespec));
	requested.tv_sec = static_cast<time_t>(nanosecs / BILLION);
	requested.tv_nsec = static_cast<long>(nanosecs % BILLION);

	timespec remaining;
	memset(&remaining, 0, sizeof(timespec));

	int result = ::nanosleep(&requested, &remaining);
	switch (result) {
	case 0:
		return;
	case -1:
		if (errno == EINTR) {
			throw Interrupt(requested.tv_nsec +
					static_cast<uint64_t>(requested.tv_sec) * BILLION);

		} else {
			throw NITRO_ERROR(errno);
		}
	default:
		// Isn't supposed to happen.
		throw NITRO_ERROR(NITRO_NOT_IMPLEMENTED);
	}
}

void Thread::sleep(uint64_t millisecs) {
	nanosleep(millisecs * 1000 * 1000);
}

void Thread::interrupt() {

}

void Thread::join() {
	// There is no locking in this method, because we should be able to
	// call it as often as we like, from as many threads as we like. The
	// underlying posix code will return immediately if the thread has already
	// terminated.
	void * exit_code_ptr = &data->exit_code;
	int rc = pthread_join(data->id, &exit_code_ptr);
	if (rc != 0) {
		throw NITRO_ERROR(rc);
	}
}

Thread::Interrupt::Interrupt(uint64_t rt) :
		std::runtime_error("Thread interrupted."),
		remaining_time(rt) {
}

} // end namespace nitro
