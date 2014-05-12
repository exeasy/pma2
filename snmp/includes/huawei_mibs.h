/*
 * huawei_mibs.h
 *
 *  Created on: Dec 25, 2012
 *      Author: jiangshan
 */

#ifndef HUAWEI_MIBS_H_
#define HUAWEI_MIBS_H_
#define DISABLE_MIB_LOADING 1


#define SYSORTABLE			".1.3.6.1.2.1.1.9"
#define SYSDES				".1.3.6.1.2.1.1.1"
#define OSPFROUTERID		".1.3.6.1.2.1.14.1.1"	//需要，可获取
#define ATTABLE				".1.3.6.1.2.1.3.1"//"IP-MIB::atTable" //
#define IPADDRTABLE			"IP-MIB::ipAddrTable"//".1.3.6.1.2.1.4.20"//需要
#define IPROUTETABLE		"RFC1213-MIB::ipRouteTable" //".1.3.6.1.2.1.4.21"			//ip路由表 需要
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
#define OSPFAREATABLE		".1.3.6.1.2.1.14.2"
#define OSPFLSDBTABLE		".1.3.6.1.2.1.14.4"	//链路状态数据库的信息，包括其域ID
#define OSPFIFMETRICTABLE 	".1.3.6.1.2.1.14.8"
#define OSPFVIRTTABLE 		".1.3.6.1.2.1.14.9"
#define OSPFNBRTABLE		".1.3.6.1.2.1.14.10"
#define OSPFIFTABLE			"OSPF-MIB::ospfIfTable"//".1.3.6.1.2.1.14.7"        //ip地址 域id hello dead时间 需要!
#define BGPPeerTable		".1.3.6.1.2.1.15.3"
#define HUAWEI_OSPF_v2_AREATABLE		".1.3.6.1.4.1.2011.5.25.155.4"      // 存在
#define HUAWEI_OSPF_v2_NEIGHBORTABLE	".1.3.6.1.4.1.2011.5.25.155.6"
#define HUAWEI_OSPF_v2_AREATABLE		".1.3.6.1.4.1.2011.5.25.155.4"
#define HUAWEI_OSPF_v2_NEIGHBORTABLE	".1.3.6.1.4.1.2011.5.25.155.4"
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
//The following OID numbers are used for querying the SNMP daemon by a client:
//zebra .1.3.6.1.2.1.4.24
//ospfd .1.3.6.1.2.1.14
//bgpd .1.3.6.1.2.1.15
//ripd .1.3.6.1.2.1.23
//ospf6d .1.3.6.1.3.102
//.iso.org.dot.internet.mgmt.mib-2.ip.ipForward
//.iso.org.dot.internet.mgmt.mib-2.ospf
//.iso.org.dot.internet.mgmt.mib-2.bgp
//.iso.org.dot.internet.mgmt.mib-2.rip2
//.iso.org.dod.internet.experimental.ospfv3

/**
 * snmpset test OID
 */
#define SETS1 ".1.3.6.1.4.1.2021.14.1.1.2.0" //s UCD-DEMO-MIB::ucdDemoPublicString.0
#define SETS2 "system.sysName.0"
#define SETI1 "snmpSetSerialNo.0"
#endif /* HUAWEI_MIBS_H_ */
