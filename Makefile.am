SUBDIRS=lib utils comm conf reg server  control  timer  mmanager vty logger snmp router lsd lsdb netlink db ddc policy test
bin_PROGRAMS=bm logsrv logcli pem threadtest snmptest icm  nltest dbtest dbm
AM_CFLAGS=-O2 -ggdb3 -std=c99 -I$(LIB_XML_INCLUDE) -I$(LIB_SNMP_INCLUDE) -lnetsnmp 
AM_LDFLAGS=-L$(LIB_SNMP_LIB)
bm_SOURCES=pma_bm.c

bm_LDADD=./comm/libpmacomm.a\
		 ./conf/libpmaconf.a\
		 ./control/libpmactl.a\
		 ./utils/libpmautils.a\
		 ./server/libpmasocket.a\
		 ./mmanager/libmodulemanager.a\
		 ./timer/libpmatimer.a\
		 ./logger/libploggersrv.a\
		 ./logger/libplogger.a\
		 ./lib/libpma.a\
		 ./reg/libpmareg.a\
		 ./vty/libpmavty.a
#bm_CFLAGS=-I/usr/include/libxml2 -D BMODULE -D OSPF_VERSION -D LOCAL_TEST -std=c99 -O2 -g
#bm_LDFLAGS=-lpthread -lxml2

dbm_SOURCES=pma_dbm.c
dbm_LDADD=./server/libpmadbm.a\
		  ./server/libpmasocket.a\
		 ./control/libpmactl.a\
		 ./logger/libplogger.a\
		 ./db/libpmadb.a\
		 ./ddc/libpmaddc.a\
		 ./policy/libpmapolicy.a\
		 ./timer/libpmatimer.a\
		 ./lib/libpma.a\
		  ./utils/libpmautils.a
dbm_CFLAGS=$(AM_CFLAGS) -D LINUX_VERSION
#dbm_CFLAGS=-I/usr/include/libxml2 -lxml2 -D DBMODULE -D OSPF_VERSION  -std=c99 -O2 -g

icm_SOURCES=pma_icm.c
icm_LDADD=./server/libpmaicm.a\
		  ./server/libpmasocket.a\
		  ./router/libpmarouter.a\
		  ./lsd/libpmalsd.a\
		  ./lsdb/libpmalsdb.a\
		  ./netlink/libpmanetlink.a\
		  ./snmp/libsnmpapi.a\
		 ./control/libpmactl.a\
		 ./logger/libplogger.a\
		 ./lib/libpma.a\
		  ./utils/libpmautils.a
#icm_CFLAGS=-I/usr/include/libxml2  -L/usr/local/snmp/lib -lnetsnmp -lpthread -D ICMODULE -D OSPF_VERSION -std=c99 -O2 -g 
#icm_CFLAGS=-I/usr/include/libxml2 -Wl -( ./server/libpmasocket.a ./lsd/libpmalsd.a ./lsdb/libpmalsdb.a -) -L/usr/local/snmp/lib -lnetsnmp -lpthread -D ICMODULE -D OSPF_VERSION -std=c99 -O2 -g 

pem_SOURCES=pma_pem.c
pem_LDADD=./server/libpmapem.a\
		  ./server/libpmasocket.a\
		 ./control/libpmactl.a\
		 ./logger/libplogger.a\
		 ./lib/libpma.a\
		 ./snmp/libsnmpapi.a\
		  ./utils/libpmautils.a
#pem_CFLAGS=-I/usr/include/libxml2 -L/usr/local/snmp/lib -lnetsnmp -D PEMODULE -D OSPF_VERSION -std=c99 -O2 -g

threadtest_SOURCES=threadtest.c
threadtest_LDADD=./lib/libpma.a\
				 ./utils/libpmautils.a
#threadtest_CFLAGS=-std=c99 -O2 -g -lpthread

snmptest_SOURCES=snmptest.c
snmptest_LDADD=./router/libpmarouter.a\
			   ./snmp/libsnmpapi.a\
			   ./lib/libpma.a\
			   ./utils/libpmautils.a
#snmptest_CFLAGS=-std=c99 -O2 -g -L/usr/local/snmp/lib -lpthread -lnetsnmp

lsdbtest_SOURCES=lsdbtest.c
lsdbtest_LDADD=./lsdb/libpmalsdb.a\
			   ./lib/libpma.a\
			   ./utils/libpmautils.a
#lsdbtest_CFLAGS=-std=c99 -O2 -g 


logsrv_SOURCES=logsrv.c
logsrv_LDADD=./logger/libploggersrv.a\
			 ./logger/libplogger.a\
			 ./conf/libpmaconf.a\
			 ./comm/libpmacomm.a\
			 ./control/libpmactl.a\
			 ./utils/libpmautils.a\
			 ./mmanager/libmodulemanager.a\
			 ./timer/libpmatimer.a\
			 ./lib/libpma.a
#logsrv_CFLAGS=-I/usr/include/libxml2  -std=c99 -O2 -g
#logsrv_LDFLAGS=-lpthread -lxml2

logcli_SOURCES=logcli.c
logcli_LDADD=./logger/libplogger.a\
			 ./utils/libpmautils.a\
			 ./lib/libpma.a
#logcli_CFLAGS= -O2 -g

nltest_SOURCES=nltest.c
nltest_LDADD=./netlink/libpmanetlink.a\
			 ./router/libpmarouter.a\
			 ./snmp/libsnmpapi.a\
			 ./utils/libpmautils.a
dbtest_SOURCES=dbtest.c
dbtest_LDADD=./db/libpmadb.a\
			 ./utils/libpmautils.a
routeaddtest_SOURCES=routeaddtest.c
routeaddtest_LDADD=./utils/libpmautils.a
testxml_SOURCES=test_xml.c
testxml_LDADD=./comm/libpmacomm.a\
			  ./conf/libpmaconf.a\
			  ./reg/libpmareg.a\
			  ./mmanager/libmodulemanager.a\
			  ./timer/libpmatimer.a\
			  ./control/libpmactl.a\
			 ./utils/libpmautils.a

EXTRA_DIST=config/* \
		   pma.conf\
		   interface.cfg\
		   pmalog.conf\
		   pma.db\
		   test