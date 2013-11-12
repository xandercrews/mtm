#include <mutex>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "base/guid.h"
#include "base/dbc.h"

/**
 * Depending on the value of RAND_MAX, rand_r may generate 16- or 32-bit
 * numbers. Normalize to something we can count on.
 */
inline uint16_t rand16(unsigned int * state) {
	return static_cast<uint16_t>(rand_r(state));
}

void generate_guid(char * buf, size_t buflen) {
	PRECONDITION(buflen >= GUID_BUF_LEN);
	static std::mutex my_mutex;
	std::lock_guard<std::mutex> lock(my_mutex);
	static unsigned int state = 0;
	if (state == 0) {
		state = time(0);
	}
	sprintf(buf, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
		    rand16(&state), rand16(&state),        // Generates a 32-bit Hex number
		    rand16(&state),                       // Generates a 16-bit Hex number
		    ((rand16(&state) & 0x0fff) | 0x4000), // Generates a 16-bit Hex number of the form 4xxx (4 indicates the UUID version)
		    rand16(&state) % 0x3fff + 0x8000,     // Generates a 16-bit Hex number in the range [0x8000, 0xbfff]
		    rand16(&state), rand16(&state), rand16(&state));
}

inline bool is_valid_guid_noise(char c) {
	return strchr(" \r\n\t-,;.{}", c) != 0;
}

// Don't compare guids forever. Anything way beyond a normal guid's length
// is irrelevant if we're comparing text as guids.
const size_t MAX_COMPARE_COUNT = (GUID_BUF_LEN - 1) * 2;

int compare_guids(char const * a, char const * b) {
	if (a) {
		if (b) {
			auto pa = a;
			auto pb = b;
			size_t char_cnt = 0;
			bool more = true;
			while (more) {
				auto ca = *pa;
				while (ca != 0 && !isxdigit(ca)) {
					if (!is_valid_guid_noise(ca)) {
						ca = 0;
						more = false;
						break;
					}
					ca = *++pa;
				}
				auto cb = *pb;
				while (cb != 0 && !isxdigit(cb)) {
					if (!is_valid_guid_noise(cb)) {
						cb = 0;
						more = false;
						break;
					}
					cb = *++pb;
				}
				++char_cnt;
				int n = toupper(ca) - toupper(cb);
				if (n) {
					return n < 0 ? -1 : 1;
				} else if (ca == 0) {
					return 0;
				}
				if (char_cnt > MAX_COMPARE_COUNT) {
					return 0;
				}
				++pa;
				++pb;
			}
		} else {
			return 1;
		}
	}
	return b ? -1 : 0;
}
