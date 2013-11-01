#include "gtest/gtest.h"
#include "common/lock.h"
#include "common/thread.h"
#include "common/error.h"

using namespace nitro;

TEST(LockThreadTest, lock_ctor) {
	pthread_mutex_t mutex;
	Lock lock(mutex);
}

TEST(LockThreadTest, double_lock_triggers_deadlock_error) {
	Mutex mutex;
	Lock lock(mutex);
	// Theoretically, we could use gtest's ASSERT_THROW macro here. However,
	// I want to make sure the error has a specific code, which ASSERT_THROW
	// doesn't support.
	try {
		Lock lock2(mutex);
		ADD_FAILURE() << "Expected a second attempt to lock the same mutex to"
				" trigger a deadlock error.";
	} catch (Error const & e) {
		EXPECT_EQ(NITRO_DEADLOCK, e.get_event_id());
	}
}


void * simple_thread_main(void * arg) {
	int * number = reinterpret_cast<int *>(arg);
	*number++;
}

TEST(LockThreadTest, one_thread_start_join_finish) {
	int call_count = 0;
	Thread thread;

	// Let the thread run to completion.
	thread.start(simple_thread_main, &call_count);
	thread.join();

	// Verify final state.
	ASSERT_FALSE(thread.is_alive());
	ASSERT_EQ(NULL, thread.get_exception());
	ASSERT_EQ(0, thread.get_exit_code());
}

void * pausing_thread_main(void * arg) {
	int * number = reinterpret_cast<int *>(arg);
	while (*number == 0) {
		Thread::sleep(10);
	}
}

TEST(LockThreadTest, running_thread_has_expected_attributes) {
	int test_number = 0;
	Thread thread;
	thread.set_name("fred");
	thread.start(pausing_thread_main, &test_number);
	// Give thread enough time to cycle once or twice.
	// Thread should not exit since we haven't set the variable it's testing.
	Thread::sleep(20);
	// Verify running state.
	ASSERT_TRUE(thread.is_alive());
	ASSERT_EQ(NULL, thread.get_exception());
	ASSERT_EQ(0, thread.get_exit_code());
	ASSERT_STREQ("fred", thread.get_name());

	// Cause thread to exit.
	test_number = 1;
	Thread::sleep(20);

	// Verify final state.
	ASSERT_FALSE(thread.is_alive());
	ASSERT_EQ(NULL, thread.get_exception());
	ASSERT_EQ(0, thread.get_exit_code());
}

void seed_random_generator() {
	static bool inited = false;
	if (!inited) {
		inited = true;
		srand(time(0));
	}
}

// Define some shared state that we can use from multiple threads.
struct SharedState {
	Mutex mutex;
	int active_count;
	char buf[12];
	bool failed;
};

// Define some state that we can pass to each thread. This includes some
// stuff that's shared, and some that's unique.
struct ThreadState {
	SharedState * shared_state;
	int thread_idx;
	int loop_count;
};

void * contending_thread_main(void * arg) {

	// Make some local variables that help to clarify/simplify the code.
	ThreadState & ts = *reinterpret_cast<ThreadState *>(arg);
	SharedState & ss = *ts.shared_state;
	char my_char = '0' + ts.thread_idx;
	unsigned my_randstate = time(0) + ts.thread_idx;

	// In all cases, exit this thread within 5 seconds. We expect to be
	// done much sooner, but this is a failsafe.
	time_t end_time = time(0) + 5;

	// Count how many active threads there are at any given time. If our
	// locking mechanism is working, the test that spawned all these threads
	// should always get a full count of threads right after launch, and a 0
	// count when all the threads exit.
	struct Counter {
		SharedState & ss;
		Counter(SharedState & ss) : ss(ss) {
			Lock lock(ss.mutex);
			ss.active_count += 1;
		}
		~Counter() {
			Lock lock(ss.mutex);
			ss.active_count -= 1;
		}
	} counter(ss);

	const size_t OVERWRITE_LEN = sizeof(ss.buf) - 1;

	// Keep track of how many times we go through the loop.
	for (ts.loop_count = 0; ; ++ts.loop_count) {

		// If we hit our failsafe timeout, report problem and quit.
		if (end_time < time(0)) {
			ADD_FAILURE() << "Exiting thread " << my_char << " after timeout"
					" and " << ts.loop_count << "iterations. We should have"
					" been interrupted. Something's wrong.";
			return 0;
		}

		// As soon as we declare the lock on the next line, we block until
		// the lock is acquired or until an exception is thrown.
		try {
			// For the duration of this block, we should have exclusive access
			// to the shared buffer, if locks are working.
			Lock lock(ss.mutex);

			// If some other thread found a failure, exit immediately.
			if (ss.failed) {
				return 0;
			}

			// Make a record of the text that's currently in our shared buf.
			char oldvalue[OVERWRITE_LEN + 1];
			strncpy(oldvalue, ss.buf, OVERWRITE_LEN);

			// Sleep anywhere between a single clock tick and ~1.6 ms. Anybody
			// who's trying to lock should also be frozen, if our locks are
			// working right.
			uint64_t interval = (rand_r(&my_randstate) % 32767) * 50;
			Thread::nanosleep(interval);

			// Nothing should have changed, since we hold the lock.
			if (memcmp(oldvalue, ss.buf, OVERWRITE_LEN) != 0) {
				ADD_FAILURE() << "Expected to see " << OVERWRITE_LEN << "'"
						<< my_char<< "' chars after sleeping " << interval
						<< " nanosecs, but saw \"" << ss.buf
						<< "\" instead. Locks aren't protecting shared state."
						" Exiting thread " << ts.thread_idx << " after "
						<< ts.loop_count << " iterations.";
				ss.failed = true;
				return 0;
			}

			// Now write my own digit into the buffer over and over.
			for (size_t i = 0; i < OVERWRITE_LEN; ++i) {
				ss.buf[i] = my_char;
			}

			// Turn around and verify that it's still there, exactly as written.
			for (size_t i = 0; i < OVERWRITE_LEN; ++i) {
				if (ss.buf[i] != my_char) {
					ADD_FAILURE() << "Expected to see " << OVERWRITE_LEN << "'"
							<< my_char<< "' chars immediately after writing"
							" them, but saw \"" << ss.buf << "\" instead."
							" Locks aren't protecting shared state."
							" Exiting thread " << ts.thread_idx << " after "
							<< ts.loop_count << " iterations.";
					ss.failed = true;
					return 0;
				}
			}
		} catch (Error const & e) {
			ADD_FAILURE() << e.what();
			ss.failed = true;
			return 0;
		}
	}
}

TEST(LockThreadTest, torture_test) {
	// Get ready to generate random sleep intervals in all our threads.
	seed_random_generator();

	// Create some shared state that we will either corrupt or not, depending
	// on whether our implementation is correct.
	SharedState shared_state;
	memset(shared_state.buf, 0, sizeof(shared_state.buf));
	shared_state.active_count = 0;
	shared_state.failed = false;

	// Now create a bunch of contending threads.
	Thread threads[7];
	ThreadState tstates[7];
	for (int i = 0; i < 7; ++i) {
		tstates[i].shared_state = &shared_state;
		tstates[i].thread_idx = i;
		threads[i].start(contending_thread_main, &tstates[i]);
	}

	// Allow threads to run for a little while. We expect that during this
	// period, we'll do a huge number of context switches--basically stressing
	// the mutexes, locks, and threading subsystem very hard. However, we also
	// expect that the system will never fail to lock, and will flawlessly
	// protected the state that all these threads share.
	Thread::sleep(1500);
	{
		Lock lock(shared_state.mutex);
		//EXPECT_EQ(7, shared_state.active_count);
		EXPECT_FALSE(shared_state.failed);
		auto first_char = shared_state.buf[0];
		for (int i = 1; i < sizeof(shared_state.buf); ++i) {
			ASSERT_EQ(first_char, shared_state.buf[i]);
		}
		// When threads wake up after the current lock is released, make them
		// exit early.
		shared_state.failed = true;
	} // release lock

	bool all_exited = false;

	// Wait a brief time for threads to notice our signal.
	for (int i = 0; i < 5; ++i) {
		all_exited = true;
		Thread::sleep(50);
		for (int j = 0; j < 7; ++j) {
			if (threads[j].is_alive()) {
				all_exited = false;
				break;
			}
		}
		if (all_exited) {
			break;
		}
	}

	// If we hang or we get here without all threads exiting, we've got
	// deadlock problems.
	EXPECT_TRUE(all_exited);
	EXPECT_EQ(0, shared_state.active_count);

	// Make sure we really created some serious contention.
	size_t total_iterations = 0;
	for (int i = 0; i < 7; ++i) {
		total_iterations += tstates[i].loop_count;
	}
	if (total_iterations < 10000) {
		ADD_FAILURE() << "Expected lots of iterations inside the loops on our"
				" threads, but only saw " << total_iterations << ".";
	}
}
