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

engine_handle make_leader_engine(char const ** args = NULL, int argc = 0) {
	char const * def_args[] = {"progname", "--passiveport", "52500",
			"--activeport", "52501"};
	if (!args) {
		args = def_args;
		argc = 5;
	}
	cmdline cmdline(argc, args);
	if (!cmdline.get_errors().empty()) {
		for (auto e: cmdline.get_errors()) {
			ADD_FAILURE() << e.what();
		}
	}
	return factory.make(cmdline);
}

engine_handle make_follower_engine(char const ** args = NULL, int argc = 0) {
	char const * def_args[] = {"progname", "--passiveport", "52502",
			"--activeport", "52503", "--leader", "tcp://localhost:52500"};
	if (!args) {
		args = def_args;
		argc = 7;
	}
	cmdline cmdline(argc, args);
	if (!cmdline.get_errors().empty()) {
		for (auto e: cmdline.get_errors()) {
			ADD_FAILURE() << e.what();
		}
	}
	return factory.make(cmdline);
}

TEST(engine_test, leader_ctor_works) {
	auto engine = make_leader_engine();
	ASSERT_TRUE(static_cast<bool>(engine));
	ASSERT_FALSE(engine->is_follower());
}

TEST(engine_test, follower_ctor_works) {
	auto engine = make_leader_engine();
	ASSERT_TRUE(static_cast<bool>(engine));
	ASSERT_FALSE(engine->is_follower());
}

TEST(engine_test, respond_to_ping) {

}
