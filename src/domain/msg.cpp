#include <string.h>

#include "base/error.h"
#include "base/guid.h"

#include "domain/event_codes.h"
#include "domain/msg.h"

#include "json/json.h"

#include "zeromq/include/zmq.h"

using namespace nitro::event_codes;

namespace nitro {

std::string serialize_msg(int eid) {
	Json::Value root;
	char guid[GUID_BUF_LEN];
	generate_guid(guid, sizeof(guid));
	root["messageId"] = guid;
	root["senderId"] = "nitro@localhost"; // TODO: fix
	root["body"]["code"] = events::get_std_id_repr(eid);
	Json::StyledWriter writer;
	return writer.write(root);
}

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
	std::string full;
	int64_t more = 1;
	size_t more_size = sizeof more;
	const size_t MAX_SIZE_WERE_WILLING_TO_RECEIVE = 1024 * 1024 * 8;
	do {
	    /* Create an empty ØMQ message to hold the message part */
	    zmq_msg_t part;
	    int rc = zmq_msg_init(&part);
	    if (rc) return full;
	    /* Block until a message is available to be received from socket */
	    rc = zmq_msg_recv(&part, socket, 0);
	    if (rc == -1) {
	    	zmq_msg_close(&part);
	    	return full;
	    }
	    auto part_size = zmq_msg_size(&part);
	    auto new_size = full.size() + part_size;
	    if (new_size > MAX_SIZE_WERE_WILLING_TO_RECEIVE) {
	    	zmq_msg_close(&part);
	    	throw ERROR_EVENT(NITRO_MSG_TOO_BIG_1SIZE, new_size);
	    }
	    auto data = reinterpret_cast<const char *>(zmq_msg_data(&part));
	    full.append(data, part_size);
	    /* Determine if more message parts are to follow */
	    rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
	    if (rc) more = false;
	    zmq_msg_close(&part);
	} while (more);
	return full;
}

void send_full_msg(void * socket, std::string const & txt) {
	const size_t BYTES_PER_FRAME = 256 * 1024;
	auto bytes_remaining = txt.size();
	auto ptr = txt.c_str();
	do {
		auto bytes_this_time = std::min(bytes_remaining, BYTES_PER_FRAME);
		bytes_remaining -= bytes_this_time;
		auto flags = (bytes_remaining ? ZMQ_SNDMORE : 0);
		zmq_msg_t msg;
		int rc = zmq_msg_init_size(&msg, bytes_this_time);
		if (rc) {
			zmq_msg_close(&msg);
			throw ERROR_EVENT(errno);
		}
		auto data = zmq_msg_data(&msg);
		memcpy(data, ptr, bytes_this_time);
		ptr += bytes_this_time;
		zmq_msg_send(&msg, socket, flags);
		zmq_msg_close(&msg);
	} while (bytes_remaining > 0);
}

} // end namespace nitro
