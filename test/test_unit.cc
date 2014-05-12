extern "C" {
#include <utils/utils.h>
#include <utils/common.h>
#include <stdio.h>
#include <conf/conf.h>
}
#include <gtest/gtest.h> 
TEST(conf_init, return_check)
{
	EXPECT_EQ(0,conf_init())<<"Conf Init test";
}

TEST(get_pma_id, return_check)
{
	EXPECT_EQ(1,get_pma_id());
}
TEST(get_version, return_check)
{
	EXPECT_EQ(3,get_version());
}

TEST(getserver_address,value_check)
{
	EXPECT_STREQ("192.168.3.248",get_server_address());
}
TEST(getserver_port,value_check)
{
	EXPECT_EQ(9181,get_server_port());
}
