#ifndef PMA_HANDLER_H
#define PMA_HANDLER_H

struct router_list{
	int rid;
	int ip_addr;
	struct router_list* next;
};

//OPS_PMS_POLICY_SEND
int handle_ops_pms_policy_send(void *);
//OPS_PMS_MARI_SEND
int handle_ops_pms_mari_send(void *);
//NEIGHBOR_LIST
int handle_ops_pms_neighbor_list(void *);
//TC_TO_PMA_INTERFACE_INFO
//TC_TO_PMA_FLOW_INFO
int handle_trans_message(void *);
//OSPF_SPF
int handle_ospf_spf_signal_ic_to_dbm(void *);
//TC_CMD_PMS_TO_PEA
int handle_ops_pms_tc_cmd(void *);
//BGP_TO_PMA_UPDATE_MESSAGE
//BGP_TO_PMA_RIB
//BGP_TO_PMA_CONF
//BGP_NEIGHBOR
//OPS_PMA_SNAPSHOT_SEND
//OPS_PMA_TRANS_PACKET
int handle_trans_packet_to_pms(void *);

//OPS_PMS_LOGIN_RESP_OK
int handle_ops_pms_login_resp_ok(void *);
//OPS_PMS_LOGIN_FAILED
int handle_ops_pms_login_failed(void *);
//OPS_ACK_FAILED
int handle_ops_ack_failed(void *);
//OPS_ACK
int handle_ops_ack(void *);

//show packet(FOR DEBUG)
int handle_packet_show_content(void *args);
//MODREG
int handle_local_module_register(void *args);
//LSDB_INFO
int handle_lsdb_snapshoot_dbm_to_pms(void *args);
//NETWORK_INFO
int handle_network_info_dbm_to_pms(void *args);
//ADDLSA
int handle_add_lsa_ic_to_dbm(void *args);
//UPDATELSA.
int handle_update_lsa_ic_to_dbm(void *args);
//ADDROUTE
int handle_add_route_ic_to_pms(void *args);
//POLICY_INFO
int handle_send_policy_dbm_to_pea(void *args);
//UP_ROUTE_INFO
int handle_route_table_ic_to_pms(void *args);
//UP_INTERFACE_INFO
int handle_tc_message_to_pms(void *args);


int save_router_list_to_xml();
int send_router_list_to_ic();


//#ifdef ICMODULE
//// IC_CONF
//int ic_conf_handle(void *);
//// PMA_LIST
//int pma_list_handle(void *);
//// ACK
//int ack_handle(void *);
//#endif
//
//#ifdef DBMODULE
//// DBM_CONF
//int dbm_conf_handle(void *);
//// ADDLSA
//// UPDATELSA
//int dbm_lsdb_handle(void *);
//// ADDROUTE
//int dbm_add_route_handle(void *);
//// UPDATEROUTE
//int dbm_update_route_handle(void *);
//// POLICY_LIST
//int dbm_update_policy_handle(void *);
//// UP_ROUTE_INFO
//int dbm_upload_route_handle(void *);
//
//// ADDROUTE
//int dbm_add_route_handle(void *);
//// UPDATEROUTE
//int dbm_update_route_handle(void *);
//// POLICY_LIST
//int dbm_update_policy_handle(void *);
//// UP_ROUTE_INFO
//int dbm_upload_route_handle(void *);
//
//// ADDROUTE
//int dbm_add_route_handle(void *);
//// UPDATEROUTE
//int dbm_update_route_handle(void *);
//// POLICY_LIST
//int dbm_update_policy_handle(void *);
//// UP_ROUTE_INFO
//int dbm_upload_route_handle(void *);
//// OSPF_SPF
//int dbm_spf_handle(void *);
//// ACK
//int ack_handle(void *);
//#endif
//
//#ifdef PEMODULE
//// POLICY_INFO
//int pem_policy_execute_handle(void *);
//// BGP_MRAI
//int pem_mrai_execute_handle(void *);
//// ACK
//int ack_handle(void *);
//// PEM_CONF
//int pem_conf_handle(void *);
//#endif
#endif
