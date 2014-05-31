#ifndef SRV_CONF_H
#define SRV_CONF_H

#define PMA_VERSION 3



#define M_STATUS_TOTAL 8
#define M_STATUS_INITING 0
#define M_STATUS_INITED 1
#define M_STATUS_LOGGING 2
#define M_STATUS_LOGGED 3
#define M_STATUS_WAITING 4
#define M_STATUS_READY	5
#define M_STATUS_WORKING 6
#define M_STATUS_OFFLINE 7

/* add,module macro by Macro.Z 2013-01-23 begin */
#define ICM 1
#define DBM	2
#define PEM	3
#define BM	4
#define PMS 5
/*Macro.Z 2013-01-23 end*/


/* pkt type by Macro.Z 2013-01-23 begin */
#define MODREG 1
#define EVENTREG 2
#define HEARTPKT 3
#define ACK 4
/*Macro.Z 2013-01-23 end*/


#define ADDLSA 11
#define UPDATELSA 12
#define ADDROUTE 13
#define UPDATEROUTE 14
#define UP_DEVICE_INFO 15
#define UP_ROUTE_INFO 16
#define UP_TRAFFICE_INFO 17
#define UP_OSPF_INTERFACE_INFO 18
#define UP_BGP_INTERFACE_INFO 19
#define UP_BGP_PATH_TABLE_INFO 20

#define NETWORK_INFO 21
#define LSDB_INFO	22
#define POLICY_INFO 231



#define OSPF_SPF 31
#define POLICY_REQUEST 32


#define POLICY_LIST  51
#define NETINFO_REQ 52
#define SNAPSHOOT_REQ 53
#define PMA_LIST 54
#define ICM_CONF 55
#define DBM_CONF 56
#define PEM_CONF 57

#define BGP_MRAI    60
#define TC_TUNNEL_COMMAND 61



#define LSDB_XML_FILE "./lsdb.xml"
#define MAX_LSDB_SIZE 204800
#define MAX_RT_SIZE 102400

#endif

