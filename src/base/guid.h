#ifndef _BASE_GUID_H_
#define _BASE_GUID_H_

#include <stddef.h> // for size_t
#include <string>

/**
 * Generating guids is not rocket science. On most linux distros, you can just
 * cat /proc/sys/kernel/random/uuid, or link to libuuid. However, I decided
 * to provide my own impl for portability and to eliminate fork+exec or
 * library dependencies.
 */

const size_t GUID_BUF_LEN = 37;

/**
 * Generate an RFC 4122-compliant guid and write it to a buffer, with null
 * termination.
 *
 * Sample output: f81d4fae-7dec-11d0-a765-00a0c91e6bf6.
 *
 * @param buf Receives the guid.
 * @param buflen Cannot be less than GUID_BUF_LEN. If this constraint is
 *     violated, an error(E_PRECONDITION_1EXPR_VIOLATED) is thrown.
 */
void generate_guid(char * buf, size_t buflen);

/**
 * Generate a guid and return it as a std::string.
 */
std::string generate_guid();

/**
 * Compare guids, and return 0 if ==, -1 if a < b, or 1 if a > b.
 *
 * Guids compare in a way that ignores case, whitespace, and punctuation, which
 * is why this function is more than just strcmp.
 */
int compare_guids(char const * a, char const * b);

#endif // sentry
