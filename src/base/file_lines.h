#ifndef _BASE_FILE_LINES_H_
#define _BASE_FILE_LINES_H_

#include <stddef.h>

/**
 * Iterate over all lines in a text file in a way that's transparent to
 * file buffering issues.
 *
 * @note This class is NOT threadsafe. That doesn't mean you can't use a single
 *     instance of it from multiple threads--it just means that if you do
 *     it, you have to mutex, because it won't do that for you. In
 *     this respect, the class's guarantees are identical to those of stl
 *     containers or std::string.
 */
class file_lines {
	struct data_t;
	data_t * data;

public:
	/**
	 * Open a file that contains a series of lines.
	 *
	 * @param gulp_size How much data should we read from the file at a time?
	 *     This parameter is primarily exposed for testing; internal caching
	 *     is already efficient, and ridiculously large values or small values
	 *     will be adjusted to something reasonable. Ordinary callers can
	 *     ignore.
	 */
	file_lines(char const * fname, size_t gulp_size=0);
	virtual ~file_lines();

	/**
	 * Return the next non-empty, non-comment line -- or NULL if the file is
	 * exhausted. Lines are automatically trimmed on both ends. Also slides
	 * internal buffer cursor forward.
	 *
	 * @return line or NULL
	 *
	 * @throws runtime_error if a line is so severely malformed that the file
	 *     must be abandoned. Right now, the only example of this is text that
	 *     runs on for a full megabyte without a line break; other validation
	 *     is the responsibility of the caller.
	 */
	char const * next();

	/**
	 * How much of the file have we consumed? Ratio is in the range [0..1].
	 */
	double ratio_complete() const;
};

#endif // sentry
