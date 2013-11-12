#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <stdio.h>
#include <string.h>

using std::min;
using std::max;

#include "base/interp.h"
#include "base/error.h"
#include "base/event_codes.h"
#include "base/file_lines.h"

using namespace base::event_codes;

/**
 * How much will we read from a file_lines file at any given time?
 *
 * Don't set this too high -- we want to be able to run multiple batches in
 * parallel, so we may have multiples of this class active at the same time.
 * Besides, past experience with file I/O shows that as long as you're reading
 * large chunks (say, >256 k), the performance doesn't change much as the
 * number grows. This is because files are often fragmented on disk, or are
 * read from the network inefficiently -- so asking for ever larger chunks of
 * them just shifts the burden of managing head movement and caching back to
 * the kernel and network I/O layers, without changing the speed, and with
 * significant increases in RAM requirements. Since we have to handle chunking
 * inside our code anyway (to cope with file_lines file sizes in the tens or
 * hundreds of gigabytes), the tradeoff doesn't make sense.
 */
const size_t MAX_GULP_SIZE = 1024 * 1024;

/**
 * This is the size of a single block read in most file systems; anything
 * smaller is totally unreasonable.
 */
const size_t MIN_GULP_SIZE = 64 * 1024;

// Undocumented but used by unit tests to artificially test boundaries without
// having to use massive amounts of RAM and do lots of I/O.
const size_t TESTING_GULP_SIZE = 40;

static char * last_line_break(char * p, char const * begin);
static bool data_seems_binary(char const * begin, char const * end);

struct file_lines::data_t {

	std::string fname;
	FILE * f;
	// Because we've defined _FILE_OFFSET_BITS=64 to gcc in the build, all file
	// I/O in our app will be 64-bit compatible. That's why we use 64-bit
	// numbers here.
	uint64_t flen;
	uint64_t foffset;
	const size_t gulp_size;
	char * buf;
	size_t buf_offset;
	size_t buf_filled_count;
	size_t gulp_count;
	size_t line_num;
	bool ltrim;
	bool rtrim;

	data_t(char const * _fname, bool ltrim, bool rtrim, size_t gulp_size) :
		fname(_fname), f(NULL),	flen(0), foffset(0),
		gulp_size(gulp_size ?
				(gulp_size == TESTING_GULP_SIZE ?
						TESTING_GULP_SIZE :
						max(MIN_GULP_SIZE, min(MAX_GULP_SIZE, gulp_size))) :
				MAX_GULP_SIZE),
		buf(NULL), buf_offset(0), buf_filled_count(0), gulp_count(0),
		line_num(0), ltrim(ltrim), rtrim(rtrim) {

		f = fopen(_fname, "r");
		if (f) {
			fseek(f, 0, SEEK_END);
			flen = ftell(f);
			if (flen) {
				fseek(f, 0, SEEK_SET);
				gulp();
			} else {
				cleanup();
				throw ERROR_EVENT(E_INPUT_FILE_1PATH_EMPTY, _fname);
			}
		} else {
			throw ERROR_EVENT(E_INPUT_FILE_1PATH_UNREADABLE, _fname);
		}
	}

	~data_t() {
		cleanup();
	}

	void cleanup() {
		if (f) {
			fclose(f);
			f = NULL;
		}
		if (buf) {
			delete[] buf;
			buf = NULL;
		}
	}

	/**
	 * Read another chunk of buf from the file. This should only be called
	 * when the current buffer is empty.
	 */
	bool gulp() {
		if (f == NULL) {
			return false;
		}
		++gulp_count;

		// Make sure we have a buffer to hold what we're going to read.
		if (buf == NULL) {

			// For small files, don't allocate a full-sized buffer. That would
			// be wasteful. We allocate flen + 1 because we want to always
			// guarantee that our buffer is null-terminated, and if the file
			// doesn't end with a line-break, that means the final byte of the
			// file isn't a place we can overwrite. So for small files, the
			// extra byte is necessary.
			buf = new char[min(flen + 1, gulp_size)];
		}

		// How much is left to read in the file?
		uint64_t file_bytes_remaining = flen - foffset;

		// Read as much as we can swallow right now, still allowing for a null
		// terminator.
		uint64_t intended_bytes_read = min(file_bytes_remaining,
				gulp_size - 1);

		uint64_t bytes_read = fread(buf, 1, intended_bytes_read, f);

		// Check for binary file on first gulp.
		if (gulp_count == 1) {
			if (data_seems_binary(buf, buf + min(bytes_read,
					static_cast<uint64_t>(100)))) {
				cleanup();
				throw ERROR_EVENT(E_1FILE_BAD_SEEMS_BINARY, fname);
			}
		}

		// Reset buffer state variables.
		buf_offset = 0;
		buf_filled_count = bytes_read;

		// Make sure our buffer is null-terminated. At the time we call this,
		// dest + bytes_read is guaranteed to be a valid location for us to
		// write (it's not past the end of our buffer). We may write our
		// null there--but only if we've exhausted our file. Otherwise, we
		// write a null on top of the final line break among the stuff we've
		// read.
		uint64_t bytes_remaining_after_read = file_bytes_remaining - bytes_read;
		bool file_exhausted = (bytes_remaining_after_read == 0);
		char * end = file_exhausted ?
				buf + bytes_read :
				last_line_break(buf + bytes_read - 1, buf);

		// One bizarre corner case to handle is when we have a single line of
		// text that's longer than GULP_SIZE - 1. That should never happen, but
		// if someone gives us malformed input, we don't want to seg fault.
		// Throw an exception instead.
		if (end == buf) { // file's not exhausted, full buffer had 0 LF
			cleanup();
			throw ERROR_EVENT(E_1FILE_BAD_HUGE_LINE_2BYTES, fname, bytes_read);
		}

		// Guarantee null termination.
		*end = 0;

		// Reset file cursor to offset of our null char, so our next read picks
		// up on a line boundary. This is slightly inefficient and could be
		// optimized, but its virtue is that it guarantees that we don't have
		// to memmove() some trailing stuff from our last read into buf; we
		// always read on line break boundaries.
		if (!file_exhausted) {
			// If what we read didn't end on a line break...
			if (end < buf + bytes_read) {
				auto extra_bytes = (buf + bytes_read) - (end + 1);
				fseek(f, -extra_bytes, SEEK_CUR);
				foffset = ftell(f);
				buf_filled_count -= extra_bytes;
			}
		} else {
			// Close file handle immediately rather than keeping it open
			// while we process the final lines of the file. This is not
			// the same as calling cleanup() -- we still want the data that's
			// in buf...
			fclose(f);
			f = NULL;
		}
		return true;
	}
};

inline char * start_of_next_line(char * buf) {
	auto c = *buf;
	switch (c) {
	case 0:
		return 0;
	case '\n':
		return buf + 1;
	case '\r':
		return (buf[1] == '\n') ? buf + 2 : buf + 1;
	default:
		return buf;
	}
}

inline void rtrim_line(const char * start, char * end) {
	// end points to null terminator or to line break char when we enter.
	// When we finish, we want it to point to a place where we can write
	// a null byte.
	do {
		--end;
	} while (end >= start && isspace(*end));
	// It's possible for end to be before start when we get here, if end
	// equaled start at the top of our loop. But underflow is harmless
	// because we're going to write one byte *after* end. That's guaranteed
	// to be a safe place--either because it was the original terminating
	// null, a line break char, or a trailing whitespace char that we can
	// clobber.
	*++end = 0;
}

size_t file_lines::get_current_line_num() const {
	return data->line_num;
}

inline char * ltrim(char * txt) {
	while (isspace(*txt)) {
		++txt;
	}
	return txt;
}

char const * file_lines::next() {
	if (data->buf) {

		auto end_of_current_gulp = data->buf + data->buf_filled_count;
		auto start = data->buf + data->buf_offset;

		// Skip over previous null terminator, if applicable.
		if (start < end_of_current_gulp && *start == 0) {
			++start;
		}

		while (true) {
			if (start && start < end_of_current_gulp) {
				auto end = strpbrk(start, "\n\r");
				if (end == NULL) {
					end = strchr(start, 0);
				}

				// Handle windows-style CR+LF.
				auto double_break = false;
				if (*end == '\r') {
					if (end[1] == '\n') {
						double_break = true;
						++end;
					}
				}

				// Trim and null terminate as needed. We do trimming here not
				// because it's necessary, but because it's more efficient than
				// trimming after we return the string.
				*end = 0;
				if (data->rtrim) {
					rtrim_line(start, end);
				} else {
					if (double_break) {
						end[-1] = 0;
					}
				}
				if (data->ltrim) {
					start = ltrim(start);
				}

				data->buf_offset = end - data->buf;
				++data->line_num;
				return start;

			} else {
				if (!data->gulp()) {
					return NULL;
				}
				start = data->buf;
				end_of_current_gulp = data->buf + data->buf_filled_count;
			}
		}
	}
	return NULL;
}

double file_lines::ratio_complete() const {
	if (data->flen == 0 || data->buf == NULL) {
		return 1.0;
	}
	return (data->foffset - data->buf_filled_count + data->buf_offset) /
			static_cast<double>(data->flen);
}

file_lines::file_lines(char const * fname, bool ltrim, bool rtrim,
		size_t gulp_size) : data(0) {
	if (fname && *fname) {
		data = new data_t(fname, ltrim, rtrim, gulp_size);
	} else {
		throw ERROR_EVENT(E_BAD_FNAME_NULL_OR_EMPTY);
	}
}

file_lines::~file_lines() {
	delete data;
}

bool data_seems_binary(char const * begin, char const * end) {
	for (char const * p = begin; p < end; ++p) {
		char c = *p;
		if (c == 127) return true;
		if (c < ' ') {
			if (c != '\t' && c != '\n' && c != '\r') {
				return true;
			}
		}
	}
	return false;
}

char * last_line_break(char * p, char const * begin) {
	do {
		char c = *p;
		if ((c == '\n') || (c == '\r')) {
			return p;
		}
	} while (--p > begin);
	return p;
}


