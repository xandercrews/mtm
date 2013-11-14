#ifndef _BASE_NETUTIL_H_
#define _BASE_NETUTIL_H_

/**
 * Get the string representation of the i-th ipaddress on the local machine.
 * If i exceeds number of addresses, NULL is returned.
 * This function is threadsafe.
 */
char const * get_local_ipaddr(int i);

#endif
