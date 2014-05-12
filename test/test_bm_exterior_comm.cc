extern "C" {
#include <utils/utils.h>
#include <utils/common.h>
#include <stdio.h>
#include <comm/comm.h>
#include <comm/exterior_daemon.h>
}
#include <gtest/gtest.h> 

int fd = 0;
TEST(create_connect, return_check)
{
	EXPECT_GT( 2, fd = create_connect("192.168.3.248",9181)); 
}

TEST(update_pma_addr, value_check)
{
	EXPECT_EQ(0, update_pma_addr(fd));
}
