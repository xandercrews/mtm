#ifndef _BASE_FILE_LINES_H_
#define _BASE_FILE_LINES_H_

#include <stddef.h>

/**
 * Iterate over all lines in a text file in a way that's transparent to
 * file buffering issues.
 *
 * You should be open a file of virtually any size with this class; it will
 * buffer intelligently (typically in multi-megabyte chunks). The only caveat
 * is that you need to be prepared to catch exceptions for files that are
 * binary, files that don't exist, or files that have a single line of text
 * longer than the maximum buffered size.
 *
 * Differences between line breaking on *nix, windows, and old-style mac are
 * handled transparently.
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
	 * @param ltrim
	 *     left-trim lines as they're returned?
	 * @param rtrim
	 *     right-trim lines as they're returned?
	 * @param gulp_size
	 *     How much data should we read from the file at a time?
	 *     This parameter is primarily exposed for testing; internal caching
	 *     is already efficient, and ridiculously large values or small values
	 *     will be adjusted to something reasonable. Ordinary callers should
	 *     ignore. Note that gulp_size is also the maximum size of an
	 *     individual line; lines that exceed gulp_size will trigger an
	 *     exception.
	 */
	file_lines(char const * fname, bool ltrim=false, bool rtrim=false,
			size_t gulp_size=0);

	virtual ~file_lines();

	/**
	 * Return the next line -- or NULL if the file is exhausted. Slides internal
	 * buffer forward unless we're at EOF.
	 *
	 * Lines may be trimmed on either or both end, depending on how the object
	 * was constructed. Lines never end with a trailing CR/LF, regardless of
	 * how lines are delimited in the file.
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
	 * Return the 1-based line number that was most recently returned. Before
	 * any lines have been read, returns 0. After file has been exhausted,
	 * continues to return final line num of the file.
	 */
	size_t get_current_line_num() const;

	/**
	 * How much of the file have we consumed? Ratio is in the range [0..1].
	 */
	double ratio_complete() const;
};

#endif // sentry
