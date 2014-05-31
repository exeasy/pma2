#ifndef PMA_HEADER_H
#define PMA_HEADER_H

/* Add device type by Macro.Z 2013-03-18 begin */
#define DEVICETYPE_APP_BM 100
#define DEVICETYPE_APP_BGPMON 101
#define DEVICETYPE_APP_TCMON 102
/*Macro.Z 2013-03-18 end*/

/*!
 * “数据包类型定义”
 */
#define OPS_PMA_LOGIN			1
#define OPS_PMS_LOGIN_ACK		2
#define OPS_PMA_INIT_REQUEST		3
#define OPS_PMS_PMA_INIT_REPLY	4
#define OPS_PMA_DEVICE_INFO		5
#define OPS_PMA_ROUTER_CONF_INFO	6
#define OPS_PMA_OSPF_CONF_INFO	7
#define OPS_PMA_OSPF_LINKSTATE	8
#define OPS_PMA_OSPF_INTERFACE_INFO	9
#define OPS_PMA_BGP_CONF_INFO	10
#define OPS_PMA_BGP_LINKSTATE	11
#define OPS_PMA_BGP_INTERFACE_INFO	12
#define OPS_PMA_FLOW_INFO		13
#define OPS_PMA_TRAFFIC_INFO	14
#define OPS_PMA_HEARTBEAT		15
#define OPS_PMS_PMA_NEIGHBOR_INFO	16
#define OPS_PMS_PMA_POLICY_INFO	17
#define OPS_PMS_PMA_INFO_ACK	18
#define OPS_PMS_PMA_INFO_ERR_ACK 19
#define OPS_PMA_ROUTING_TABLE 20
#define OPS_PMA_LINKSTATE	78
#define OPS_PMA_ROUTING_TABLE 79

#define OPS_BGP_UPDATEMESSAGE 71
#define OPS_BGP_RIB			72
#define OPS_BGP_CONF		73
#define OPS_BGP_NEIGHBOR	77


#define OPS_PMS_MARI_SEND	142
#define OPS_TUNNEL_COMMAND	40
#define OPS_TUNNEL_RESULT	41


#define OPS_ACK					18
#define OPS_ACK_FAILED			19
//pma - pms
#define OPS_PMA_LOGIN			1	/*!< 代理向服务端发送的注册数据包 */
#define OPS_PMS_LOGIN_RESP_OK	2//2->9	/*!< 服务器返回的注册结果数据包，结果为“成功” */
#define OPS_PMS_LOGIN_FAILED	4	/*!< 服务器返回的注册结果数据包，结果为“失败” */
#define OPS_PMA_LOGGOUT			8	/*!< 代理向服务端发送的注销数据包 */
#define OPS_PMA_POLICY_REQ		3	/*!< 代理向服务端发送的策略请求数据包，目前并未使用 */
#define OPS_PMS_POLICY_SEND		4	/*!< 服务端向代理发送的策略数据包 */
#define OPS_PMA_NET_INFO_SEND	6	/*!< 代理向服务端发送的NETINFO数据包 */
#define OPS_PMA_SNAPSHOT_SEND	7	/*!< 代理向服务端发送的定时快照数据包 */

#define OPS_PMA_COMPLEX_DDC     60      /* 代理向CR下发流量调节因子数据包类型 */
#define COMPLEX_DDC_FLAG     3      /* 代理向CR下发策略,operation=3 */
#define OPS_OPENFLOW_FLOW_INFO_SEND	30	/* pma to pms openflow buffer */
#define OPS_PMS_FLOW_POLICY_SEND	31	/* pms to pma openflow buffer */

#define PMA_CREATE_MANAGED_INFO		51	/* communication assistant */
#define PMA_MANAGED_RESPONSE_INFO	52	/* communication assistant */
#define PMA_PMS_PULSE_INFO		53	/* communication assistant */
#define PMA_TO_PMS_MANAGED_INFO		55	/* communication assistant */
#define PMS_TO_PMA_MANAGED_RELEASED      56/*communication assistant*/

#define PMS_TO_PMA_MANAGED_INFO_RESPONSE	58	/* communication assistant */
#define PMS_TO_PMA_MANAGED_RELEASED_RESPONSE      59/*communication assistant*/
//pma - cr
#define OPS_CR_POLICY_REQ		9	/*!< CR向代理发送的策略请求数据包 */
#define OPS_PMA_POLICY_RESP		10	/*!< 代理向CR发送的策略响应数据包 */
#define OPS_PMA_NET_INFO_REQ	11	/*!< 代理向CR发送的NETINFO请求数据包 */
#define OPS_CR_NET_INFO_SEND	13	/*!< CR向代理发送的NETINFO数据包 */
#define OPS_CR_SNAPSHOT_SEND	12	/*!< CR向代理发送的定时快照数据包 */
//TC
#define TC_CMD_PMS_TO_PEA 40
#define TC_CMD_ACK_TO_PMS 41

//TCM
#define TC_TO_PMA_INTERFACE_INFO 23
#define TC_TO_PMA_FLOW_INFO 24
#define OSPF_SPF_SIGNAL 25


//BGPmon
#define BGP_TO_PMA_UPDATE_MESSAGE 71 
#define BGP_TO_PMA_RIB 72
#define BGP_TO_PMA_CONF 73
#define BGP_NEIGHBOR 77
#define BGP_LINK_INFO 78
#define OPS_PMS_MARI_SEND	142

//ALCA Part

#define PKT_TYPE_TCP 1
#define PKT_TYPE_DUP 2
#define ALCA_VERSION 1

#define NEIGHBOR_LIST 61

#define OPS_PMA_SEEK_HELP 11
#define OPS_PMA_SEEK_HELP_RESP 12
#define OPS_PMA_TRIGGER_HELP 13
#define OPS_PMA_HOSTING_ASK 14
#define OPS_PMA_TRANS_PACKET 15
#define OPS_PMA_HOSTING_RELEASE 16
#define OPS_PMA_HOSTING_SETUP 17
#define OPS_PMA_HOSTING_FAILED 18

//LogSrv
#define  PKT_TYPE_EVENT_NOTIFY 99

#define NORMAL 0
#define BEING_HOSTED 1
#define HOSTED 2
#endif
