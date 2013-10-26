/*
 * batch.h
 *
 *  Created on: Oct 25, 2013
 *      Author: dhardman
 */

#ifndef BATCH_H_
#define BATCH_H_

#include <cstring> // for size_t

namespace mtm {

/**
 * Manage a single file of commands. Iterate over them in a way that's
 * transparent to file buffering issues.
 */
class Batch {
	struct Data;
	Data * data;

public:
	/**
	 * Open a file that contains a batch of jobs/tasks.
	 *
	 * @param gulp_size How much data should we read from the file at a time?
	 *     This parameter is primarily exposed for testing; internal caching
	 *     is already efficient, and ridiculously large values or small values
	 *     will be adjusted to something reasonable. Ordinary callers can
	 *     ignore.
	 */
	Batch(char const * fname, size_t gulp_size=0);
	virtual ~Batch();

	/**
	 * Return the next non-empty, non-comment line -- or NULL if the batch is
	 * exhausted. Lines are automatically trimmed on both ends. Also slides
	 * internal buffer cursor forward.
	 *
	 * @return line or NULL
	 *
	 * @throws runtime_error if a line is so severely malformed that the batch
	 *     must be abandoned. Right now, the only example of this is text that
	 *     runs on for a full megabyte without a line break; other validation
	 *     is the responsibility of the caller.
	 */
	char const * next_line();

	/**
	 * How much of the batch have we consumed? Ratio is in the range [0..1].
	 */
	double ratio_complete() const;
};

} /* namespace mtm */

#endif /* BATCH_H_ */
