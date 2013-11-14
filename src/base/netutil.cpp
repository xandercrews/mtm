#include <mutex>
#include <string>
#include <vector>

#include <sys/types.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>

#include "base/netutil.h"

using std::mutex;
using std::lock_guard;
using std::vector;
using std::string;

char const * get_local_ipaddr(int i) {
	static vector<string> addrs;
	static bool inited = false;
	if (!inited) {
		static mutex the_mutex;
		lock_guard<mutex> lock(the_mutex);
		if (!inited) {
			inited = true;
		    struct ifaddrs *myaddrs, *ifa;
		    void *in_addr;
		    char buf[64];

		    if (getifaddrs(&myaddrs) != 0)
		    {
		        perror("getifaddrs");
		        exit(1);
		    }

		    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
		    {
		        if (ifa->ifa_addr == NULL)
		            continue;
		        if (!(ifa->ifa_flags & IFF_UP))
		            continue;

		        switch (ifa->ifa_addr->sa_family)
		        {
		            case AF_INET:
		            {
		                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
		                in_addr = &s4->sin_addr;
		                break;
		            }
#if 0
		            case AF_INET6:
		            {
		                struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
		                in_addr = &s6->sin6_addr;
		                break;
		            }
#endif

		            default:
		                continue;
		        }

		        if (inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf))) {
		            addrs.push_back(buf);
		        }
		    }

		    freeifaddrs(myaddrs);
		}
	}
	if (static_cast<size_t>(i) < addrs.size()) {
		return addrs[i].c_str();
	}
	return nullptr;
}
