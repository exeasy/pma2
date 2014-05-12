extern "C" {
#include <utils/utils.h>
#include <utils/common.h>
#include <stdio.h>
#include <conf/conf.h>
	#include <comm/comm.h>
	#include <comm/exterior_daemon.h>
	#include <comm/interior_daemon.h>
}
#include <gtest/gtest.h> 
#define CHECK_VALUE(func, y) \
	TEST(func, value_check)\
{\
	EXPECT_EQ(y, func());\
}

#define CHECK_STR(func, y) \
	TEST(func, str_check)\
{\
	EXPECT_STREQ(y, func());\
}

/* conf test */
CHECK_VALUE( conf_init , 0)
CHECK_VALUE( get_comm_type, 1)
CHECK_VALUE( get_version, 3)
CHECK_VALUE( get_pma_id, 1)
CHECK_VALUE( get_listen_port, 8025)
CHECK_STR( get_server_address, "192.168.3.248")
CHECK_VALUE( get_server_port, 9181)
CHECK_STR( get_alca_address, "127.0.0.1")
CHECK_VALUE( get_alca_port, 13456)
CHECK_STR( get_logsrv_address, "192.168.3.248")
CHECK_VALUE( get_logsrv_port, 3366)
CHECK_STR( get_rltm_address, "127.0.0.1")
CHECK_VALUE( get_rltm_port, 1900)

/* exterior_com test */
	int fd = 0;
TEST( create_connect , return_check )
{
	EXPECT_LT(0, fd = create_connect("192.168.3.248",9181));
}

  TEST( update_pma_addr, return_check)
{
	EXPECT_EQ(0, update_pma_addr(fd));
}

  TEST( close_connect, return_check)
{
	EXPECT_EQ(0, close_connect(fd));
}


  TEST(local_comm, packet_test )
{
	interior_daemon_init();
}
