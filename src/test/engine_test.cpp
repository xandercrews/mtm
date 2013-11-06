/*
 * engine_test.cpp
 *
 *  Created on: Oct 24, 2013
 *      Author: dhardman
 */

#include "gtest/gtest.h"
#include "domain/engine.h"
#include "domain/cmdline.h"

using namespace nitro;

engine_factory const & factory = engine_factory::singleton();

TEST(engine_test, leader_ctor_works) {
	char const * args[] = {"progname", "--serverport", "2500"};
	cmdline cmdline(3, args);
	engine_handle engine = factory.make(cmdline);
	ASSERT_TRUE(static_cast<bool>(engine));
	ASSERT_FALSE(engine->is_follower());
}

TEST(engine_test, follower_ctor_works) {
	char const * args[] = {"progname", "--talkport", "2500", "--follow"};
	cmdline cmdline(4, args);
	engine_handle engine = factory.make(cmdline);
	ASSERT_TRUE(static_cast<bool>(engine));
	ASSERT_TRUE(engine->is_follower());
}
