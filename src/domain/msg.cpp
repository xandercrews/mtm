#include "base/events.h"
#include "base/guid.h"

#include "domain/msg.h"

#include "zeromq/include/zmq.h"

namespace nitro {

std::string serialize_msg(int eid, std::string const & txt) {
	Json::Value root;
	char guid[GUID_BUF_LEN];
	generate_guid(guid, sizeof(guid));
	root["messageId"] = guid;
	root["senderId"] = "nitro@localhost"; // TODO: fix
	root["body"]["code"] = events::get_std_id_repr(eid);
	root["body"]["message"] = txt;
	Json::StyledWriter writer;
	return writer.write(root);
}

bool deserialize_msg(std::string const & txt, Json::Value & into) {
	Json::Reader reader;
	return reader.parse(txt, into);
}

std::string receive_full_msg(void * socket) {
	// TODO: start checking for additional frames and appending to result.
	char buf[1024];
    zmq_recv(socket, buf, sizeof(buf), 0);
    return buf;
}

void send_full_msg(void * socket, std::string const & msg) {
	// TODO: start sending in multiple frames if huge msg
	zmq_send(socket, msg.c_str(), msg.size() + 1, 0);
}

} // end namespace nitro
