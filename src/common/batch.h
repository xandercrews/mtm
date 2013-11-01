#ifndef _NITRO_COMMON_BATCH_H_
#define _NITRO_COMMON_BATCH_H_

#include <cstring> // for size_t

namespace nitro {

/**
 * Manage a single file of commands. Iterate over them in a way that's
 * transparent to file buffering issues.
 *
 * @note This class is NOT threadsafe. That doesn't mean you can't use a single
 *     instance of Batch from multiple threads--it just means that if you do
 *     it, you have to mutex, because the Batch class won't do it for you. In
 *     this respect, the class's guarantees are identical to those of stl
 *     containers or std::string. The {@link Queue} class is an example of a
 *     consumer of Batch that does mutexing correctly.
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

} /* namespace nitro */

#endif /* BATCH_H_ */
