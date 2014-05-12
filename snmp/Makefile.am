noinst_LIBRARIES=libsnmpapi.a
noinst_HEADERS=includes/bgp_snmp.h\
			   includes/huawei_mibs.h\
			   includes/typemapper.h\
			   includes/oids.h\
			   includes/pma_api.h\
			   includes/snmpget.h\
			   includes/snmpcore.h\
			   includes/snmpset.h\
			   includes/snmptable.h
libsnmpapi_a_SOURCES=snmp/getBgpRouterTable.c\
					 snmp/routerget.c\
					 snmp/routerset.c\
					 snmp/snmpget.c\
					 snmp/snmpget1.c\
					 snmp/snmpset.c\
					 snmp/snmpcore.c\
					 snmp/snmptable2.c\
					 snmp/snmptrap2.c
libsnmpapi_a_CFLAGS=-I./includes/ -I/usr/local/snmp/include -L/usr/local/snmp/lib  -std=c99
					
