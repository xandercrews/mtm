/*
 * batch.cpp
 *
 *  Created on: Oct 25, 2013
 *      Author: dhardman
 */

#include <stdexcept>
#include <cstdint>
#include <algorithm>
#include <stdio.h>

using std::min;
using std::max;

#include "common/batch.h"
#include "common/interp.h"
#include "common/error.h"
#include "common/event.h"

namespace mtm {

/**
 * How much will we read from a batch file at any given time?
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
 * inside our code anyway (to cope with batch file sizes in the tens or
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

struct Batch::Data {

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
	size_t gulp_count;

	Data(char const * _fname, size_t gulp_size) :
		fname(_fname), f(NULL),	flen(0), foffset(0),
		gulp_size(gulp_size ?
				(gulp_size == TESTING_GULP_SIZE ?
						TESTING_GULP_SIZE :
						max(MIN_GULP_SIZE, min(MAX_GULP_SIZE, gulp_size))) :
				MAX_GULP_SIZE),
		buf(NULL), buf_offset(0), gulp_count(0) {

		f = fopen(_fname, "r");
		if (f) {
			fseek(f, 0, SEEK_END);
			flen = ftell(f);
			if (flen) {
				fseek(f, 0, SEEK_SET);
				gulp();
			} else {
				fclose(f);
				f = NULL;
			}
		}
	}

	~Data() {
		if (f) {
			fclose(f);
		}
		if (buf) {
			delete[] buf;
		}
	}

	/**
	 * Read another chunk of buf from the file. This should only be called
	 * when the buf buffer is totally empty.
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

		// Read as much as is appropriate.
		uint64_t bytes_remaining = flen - foffset;
		uint64_t intended_bytes_read = min(bytes_remaining, gulp_size - 1);
		uint64_t bytes_read = fread(buf, 1, intended_bytes_read, f);

		// Make sure our buffer is null-terminated. At the time we call this,
		// buf + bytes_read is guaranteed to be a valid location for us to
		// write (it's not past the end of our buf buffer). We may write our
		// null there--but only if we've exhausted our file. Otherwise, we
		// write a null on top of the final line break among the stuff we've
		// read.
		uint64_t bytes_remaining_after_read = bytes_remaining - bytes_read;
		bool file_exhausted = (bytes_remaining_after_read == 0);
		char * end = file_exhausted ?
				buf + bytes_read :
				last_line_break(buf + bytes_read - 1, buf);

		// One bizarre corner case to handle is when we have a single line of
		// text that's longer than GULP_SIZE - 1. That should never happen, but
		// if someone gives us malformed input, we don't want to seg fault.
		// Throw an exception instead.
		if (end == buf) { // file's not exhausted, full buffer had 0 LF
			throw MTM_ERROR(MTM_1FILE_BAD_HUGE_LINE_2BYTES, fname, bytes_read);
		}

		*end = 0;
		if (gulp_count == 1) {
			if (data_seems_binary(buf, end)) {
				throw MTM_ERROR(MTM_1FILE_BAD_SEEMS_BINARY);
			}
		}

		// Reset file cursor to offset of our null char, so our next read picks
		// up on a line boundary.
		if (!file_exhausted) {
			if (end < buf + bytes_read) {
				fseek(f, (buf + bytes_read) - end, SEEK_CUR);
				foffset = ftell(f);
			}
		} else {
			// Close file handle immediately rather than keeping it open
			// while we process the final lines of the batch.
			fclose(f);
			f = NULL;
		}
		return true;
	}
};

char const * Batch::next_line() {
	return NULL;
}

double Batch::ratio_complete() const {
	if (data->flen == 0) {
		return 1.0;
	}
	return data->foffset / static_cast<double>(data->flen);
}

Batch::Batch(char const * fname, size_t gulp_size) : data(0) {
	if (fname && *fname) {
		data = new Data(fname, gulp_size);
	} else {
		throw MTM_ERROR(MTM_BAD_FNAME_NULL_OR_EMPTY);
	}
}

Batch::~Batch() {
	delete data;
}

bool data_seems_binary(char const * begin, char const * end) {
	for (char const * p = begin; p < end; ++p) {
		char c = *p;
		if (c < ' ' || c == 127) {
			if (c != '\t' && c != '\n' && c != '\r') {
				return true;
			}
		}
	}
}

char * last_line_break(char * p, char const * begin) {
	do {
		if (*p == '\n') {
			return p;
		}
	} while (--p > begin);
	return p;
}

} /* namespace mtm */
