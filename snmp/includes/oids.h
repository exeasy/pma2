#include "pma_api.h"
#ifndef OIDS_H_
#define OIDS_H_
#define DISABLE_MIB_LOADING 1

#define MAX_LEN 200

#define HUAWEI_SYSTEMNAME "Huawei"
#define FILEPATH "./tcl/"
#define FILENAME_ADDSTATICROUTE_HUAWEI 		"add_route_huawei.tcl"
#define FILENAME_SETHELLO_HUAWEI 			"set_hello_huawei.tcl"
#define FILENAME_SETDEAD_HUAWEI 			"set_dead_huawei.tcl"
#define FILENAME_SETRETRANSMIT_HUAWEI 		"set_retransmit_huawei.tcl"
#define FILENAME_SETCOST_HUAWEI 			"set_cost_huawei.tcl"
#define FILENAME_REMOVESTATICROUTE_HUAWEI 	"remove_route_huawei.tcl"
#define FILENAME_SETBGPUPTADEINTERVAL_HUAWEI "set_bgpupdateinterval_huawei.tcl"

#define FILENAME_ADDSTATICROUTE_QUAGGA 		"add_route_quagga.tcl"
#define FILENAME_SETHELLO_QUAGGA 			"set_hello_quagga.tcl"
#define FILENAME_SETDEAD_QUAGGA 			"set_dead_quagga.tcl"
#define FILENAME_SETRETRANSMIT_QUAGGA 		"set_retransmit_quagga.tcl"
#define FILENAME_SETCOST_QUAGGA 			"set_cost_quagga.tcl"
#define FILENAME_REMOVESTATICROUTE_QUAGGA 	"remove_route_quagga.tcl"
#define FILENAME_SETBGPUPTADEINTERVAL_QUAGGA "set_bgpupdateinterval_quagga.tcl"

#define FILENAME_GETIFRATE_HUAWEI 	"getIfrate.tcl"

#define SYSORTABLE			".1.3.6.1.2.1.1.9"
#define SYSDES				".1.3.6.1.2.1.1.1"
#define ATTABLE				".1.3.6.1.2.1.3.1"//"IP-MIB::atTable" //
#define IPADDRTABLE			"IP-MIB::ipAddrTable"//".1.3.6.1.2.1.4.20"//需要
#define IPROUTETABLE		"RFC1213-MIB::ipRouteTable" //".1.3.6.1.2.1.4.21"			//ip路由表 需要
#define IPFORWARDTABLE		"IP-FORWARD-MIB::ipCidrRouteTable"
//#define IPROUTETABLE		".1.3.6.1.2.1.4.21"
#define IPTOMEDIAABLE		".1.3.6.1.2.1.4.22"			//需要
#define IFTABLE				".1.3.6.1.2.1.2.2"			//接口条目列表 需要
#define INTERFACES			".1.3.6.1.2.1.2.1"			//路由器上的接口信息
#define SYSNAME				".1.3.6.1.2.1.1.5.0"
#define SYSLOCATION			"1.3.6.1.2.1.1.6.0"
#define HUAWEI				"1.3.6.1.4.1.2011.1"
#define TAB2631				"1.3.6.1.4.1.2011.10.1.54.10"
#define AA					"1.3.6.1.4.1.2011.5.25.194"
#define sysLocation			".1.3.6.1.4.1.2021.251.1"

//OSPF
#define OSPFVERSION			".1.3.6.1.2.1.14.1.3.0"
#define OSPFROUTERID		".1.3.6.1.2.1.14.1.1.0"	//需要，可获取
#define OSPFAREATABLE		"OSPF-MIB::ospfAreaTable"
#define OSPFLSDBTABLE		".1.3.6.1.2.1.14.4"	//链路状态数据库的信息，包括其域ID
#define OSPFIFMETRICTABLE 	"OSPF-MIB::ospfIfMetricTable"
#define OSPFVIRTTABLE 		".1.3.6.1.2.1.14.9"
#define OSPFNBRTABLE		"OSPF-MIB::ospfNbrTable"
#define OSPFIFTABLE			"OSPF-MIB::ospfIfTable"//".1.3.6.1.2.1.14.7"        //ip地址 域id hello dead时间 需要!
#define OSPFSPFRUNS			"1.3.6.1.2.1.14.2.1.4" //路由收敛计算次数

//BGP
#define BGPVERSION 			".1.3.6.1.2.1.15.1.0"
#define BGPLOCALAS			".1.3.6.1.2.1.15.2.0"
#define BGPPEERTABLE		".1.3.6.1.2.1.15.3"
#define BGPIDENTIFIER		".1.3.6.1.2.1.15.4.0"
#define BGPRCVDPATHATTRTABLE		".1.3.6.1.2.1.15.5"
#define BGP4PATHATTRTABLE		".1.3.6.1.2.1.15.6"


#define HUAWEI_OSPF_v2_AREATABLE		".1.3.6.1.4.1.2011.5.25.155.4"      // 存在
#define HUAWEI_OSPF_v2_NEIGHBORTABLE	".1.3.6.1.4.1.2011.5.25.155.6"
#define HUAWEI_OSPF_v2_AREATABLE		".1.3.6.1.4.1.2011.5.25.155.4"
#define HUAWEI_OSPF_v3_AREATABLE		".1.3.6.1.4.1.2011.5.25.147.1.2"
#define hwOspfv3AsLsdbTable				".1.3.6.1.4.1.2011.5.25.147.1.3"
#define hwOspfv3LinkLsdbTable			".1.3.6.1.4.1.2011.5.25.147.1.4"
#define hwOspfv3IfTable					".1.3.6.1.4.1.2011.5.25.147.1.5"
#define hwOspfv3VirtIfTable				".1.3.6.1.4.1.2011.5.25.147.1.6"
#define hwOspfv3NbrTable				".1.3.6.1.4.1.2011.5.25.147.1.7"

/**
 * The following OID numbers are used for the interprocess communication of snmpd and the
 * Quagga daemons. Sadly, SNMP has not been implemented in all daemons yet.
 *
 */
#define zebra 		".1.3.6.1.4.1.3317.1.2.1" //.gnome.gnomeProducts.zebra.zserv
#define bgpd 		".1.3.6.1.4.1.3317.1.2.2" //.gnome.gnomeProducts.zebra.bgpd
#define ripd 		".1.3.6.1.4.1.3317.1.2.3" //.gnome.gnomeProducts.zebra.ripd
#define ospfd 		".1.3.6.1.4.1.3317.1.2.5" //.gnome.gnomeProducts.zebra.ospfd
#define ospf6d 		".1.3.6.1.4.1.3317.1.2.6" //.gnome.gnomeProducts.zebra.ospf6d
/**
 * snmpset test OID
 */
#define SETS1 ".1.3.6.1.4.1.2021.14.1.1.2.0" //s UCD-DEMO-MIB::ucdDemoPublicString.0
#define SYSTEMNAME "system.sysName.0"
#define SETI1 "snmpSetSerialNo.0"

#endif /* HUAWEI_MIBS_H_ */
