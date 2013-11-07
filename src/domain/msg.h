#ifndef _DOMAIN_MSG_H_
#define _DOMAIN_MSG_H_

#include <string>

#include "json/json.h"

namespace nitro {

std::string serialize_msg(int eid, std::string const & txt);
bool deserialize_msg(std::string const & txt, Json::Value & into);

void send_full_msg(void * socket, std::string const & msg);
std::string receive_full_msg(void * socket);

}

#endif // sentry
