#GTEST_DIR = /home/macro/gtest-1.7.0
#CPPFLAGS = -isystem $(GTEST_DIR)/include

CXXFLAGS = -g -Wall -Wextra -lpthread

#noinst_LIBRARIES=libgtest.a
#
#libgtest_a_SOURCES=$(GTEST_DIR)/src/gtest-all.cc\
#				$(GTEST_DIR)/src/gtest_main.cc
#libgtest_a_CXXFLAGS=$(CPPFLAGS) -I$(GTEST_DIR)

AM_CXXFLAGS=$(CXXFLAGS) -I./include/
bin_PROGRAMS=test_bm_conf test_bm_mmanager  test_bm test_icm

test_bm_conf_SOURCES=test_bm_conf.cc
test_bm_conf_LDADD=../conf/libpmaconf.a\
				   ../utils/libpmautils.a\
				   ./libgtest_main.la
test_bm_conf_CXXFLAGS=$(AM_CXXFLAGS)
test_bm_mmanager_LDADD=../mmanager/libmodulemanager.a\
				   ../utils/libpmautils.a\
				   ../lib/libpma.a

test_bm_SOURCES=test_bm.cc
test_bm_LDADD=../comm/libpmacomm.a\
			  ../conf/libpmaconf.a\
			  ../control/libpmactl.a\
			  ../utils/libpmautils.a\
			  ../server/libpmasocket.a\
			  ../mmanager/libmodulemanager.a\
			  ../timer/libpmatimer.a\
			  ../logger/libploggersrv.a\
			  ../logger/libplogger.a\
			  ../lib/libpma.a\
			  ../reg/libpmareg.a\
			  ../vty/libpmavty.a\
			  libgtest_main.la
test_bm_CXXFLAGS=$(AM_CXXFLAGS)
test_icm_SOURCES=test_icm.cc
test_icm_LDADD=../server/libpmaicm.a\
		 ../server/libpmasocket.a\
		 ../router/libpmarouter.a\
		 ../lsd/libpmalsd.a\
		 ../lsdb/libpmalsdb.a\
		 ../netlink/libpmanetlink.a\
		 ../snmp/libsnmpapi.a\
		 ../control/libpmactl.a\
		 ../logger/libplogger.a\
		 ../lib/libpma.a\
		 ../utils/libpmautils.a\
		libgtest_main.la
test_icm_CXXFLAGS=$(AM_CXXFLAGS)
