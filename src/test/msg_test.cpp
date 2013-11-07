#include "base/guid.h"

#include "domain/msg.h"
using namespace nitro;

#include "gtest/gtest.h"


#include "test/test_util.h"

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
