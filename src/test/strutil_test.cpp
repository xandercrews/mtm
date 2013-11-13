#include "base/guid.h"
#include "base/error.h"
#include "base/strutil.h"

#include "gtest/gtest.h"

TEST(strutil_test, parse_exec_hosts) {
  std::vector<std::string> ehv;
  std::string exec_hosts("napali/0+napali/1+napali/2+eris/0+eris/1+eris/2");
  std::string exec_hosts2("napali/0-2+eris/0-2");

  parse_exec_hosts(exec_hosts, ehv);
  EXPECT_EQ(6, ehv.size());
  EXPECT_STREQ("napali", ehv[0].c_str());
  EXPECT_STREQ("napali", ehv[1].c_str());
  EXPECT_STREQ("napali", ehv[2].c_str());
  EXPECT_STREQ("eris", ehv[3].c_str());
  EXPECT_STREQ("eris", ehv[4].c_str());
  EXPECT_STREQ("eris", ehv[5].c_str());

  ehv.clear();

  parse_exec_hosts(exec_hosts2, ehv);
  EXPECT_EQ(2, ehv.size());
  EXPECT_STREQ("napali", ehv[0].c_str());
  EXPECT_STREQ("eris", ehv[1].c_str());
}

