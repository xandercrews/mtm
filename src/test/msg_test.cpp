#include "base/guid.h"

#include "domain/msg.h"
#include "domain/zmq_helpers.h"

#include "gtest/gtest.h"

#include "json/json.h"

#include "test/test_util.h"

#include "zeromq/include/zmq.h"

using std::string;
using namespace nitro;

TEST(msg_test, serialize_msg) {
	auto json = serialize_msg(0x20345096, "test \"msg");
	expect_str_contains(json, "0x20345096");
	expect_str_contains(json, "\"test \\\"msg\"");
}

TEST(msg_test, deserialize_msg) {
	auto json =
"{\n"
"  \"messageId\": \"<guid>\",\n"
"  \"messageType\": \"event\",\n"
"  \"ttl\": 3000,\n"
"  \"sentDate\": \"<when>\",\n"
"  \"senderId\": \"mwm@mordor\",\n"
"  \"body\": {\n"
"        \"code\": \"0x20394906\",\n"
"        \"message\": \"Job 123 started.\",\n"
"        \"eventType\": \"VM create\",\n"
"        \"eventDate\": \"2013-05-29 19:27:03.237+0800\",\n"
"        \"origin\": \"src/moab/MReservation.c, MCreateRes(), line 0498\",\n"
"        \"associatedObjects\": [{\"type\":\"job\", \"id\":\"12345\"}]\n"
"  }\n"
"}";
	Json::Value root;
	ASSERT_TRUE(deserialize_msg(json, root));
	EXPECT_STREQ("<guid>", root["messageId"].asCString());
	EXPECT_EQ(3000, root["ttl"].asInt());
	EXPECT_STREQ("0x20394906", root["body"]["code"].asCString());
	EXPECT_STREQ("2013-05-29 19:27:03.237+0800", root["body"]["eventDate"].asCString());
}

TEST(msg_test, send_and_receive_multipart) {

	void * ctx = zmq_ctx_new();
	zctx_cleaner z1(ctx);

	void * sender = zmq_socket(ctx, ZMQ_PUSH);
	zsocket_cleaner z2(sender);

	const char * const INPROC_ENDPOINT = "inproc://send_and_receive_multipart";
	zmq_bind_and_log(sender, INPROC_ENDPOINT);

	void * receiver = zmq_socket(ctx, ZMQ_PULL);
	zsocket_cleaner z3(receiver);

	zmq_connect_and_log(receiver, INPROC_ENDPOINT);

	const size_t TEST_MSG_SIZE = 1024 * 512 + 5;
	string sent(TEST_MSG_SIZE, 'x');
	send_full_msg(sender, sent);

	auto received = receive_full_msg(receiver);
	EXPECT_EQ(TEST_MSG_SIZE, received.size());
}
