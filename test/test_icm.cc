extern "C" {
#include <utils/utils.h>
#include <utils/common.h>
#include <stdio.h>
#include<router/router.h>
	#include <router/interface.h>
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

char routerip[24] = "192.168.3.1";
extern struct router localrouter;
TEST( update_interface_from_snmp, funcheck)
{
	localrouter.type = 1;
	EXPECT_EQ(0, update_interface_from_snmp(routerip));
}
