#include "utils/common.h"
#include "utils/utils.h"
#include <utils/xml.h>
#include "handler.h"
#include "interior_daemon.h"
#include "header.h"
#include "comm_utils.h"
#include "conf/conf.h"
#include "timer/timer.h"
#define TC_CMD_ADDR "127.0.0.1"
#define TC_CMD_PORT 8026

int router_list_timer = -1;
u32 net_info_req_timer_id = 0;
struct router_list* rList;
pthread_mutex_t router_list_mutex = PTHREAD_MUTEX_INITIALIZER;

int init_router_list()
{
	rList = (struct router_list*)malloc(sizeof(struct router_list));
	rList->ip_addr = 0;
	rList->rid = 0;
	rList->next = NULL;
	printf("INIT ROUTER LIST SUCCESS\n");
}

int find_router_by_id(int rid)
{
	struct router_list* tmp = rList->next;
	while(tmp!=NULL)
	{
		if(tmp->rid == rid)
			break;
		tmp = tmp->next;
	}
	return tmp;
}

int insert_router_list(int rid,int ip_addr)
{
	struct router_list* tmp = find_router_by_id(rid);
	if(tmp==NULL)
	{
		struct router_list* router = (struct router_list*)malloc(sizeof(struct router_list));
		router->rid = rid;
		router->ip_addr = ip_addr;
		router->next = rList->next;
		rList->next = router;
	}
	else
	{
		tmp->ip_addr = ip_addr;
	}
	return 0;
}

int handle_ops_pms_policy_send(void *args)
{
	struct packet * pkt = (struct packet*)args;
	int ack_t = OPS_ACK;
	DEBUG(INFO, "%s %d %d %u handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	pkt->data += sizeof(struct pma_pms_header);
	pkt->len -= sizeof(struct pma_pms_header);
	send_message_to_module(DBM, POLICY_LIST, pkt->data, pkt->len);
	int ret = send_ack_to_pms(pkt);
	if(ret)
		return 0;
	return -1;
}

int handle_ops_pms_mari_send(void *args)
{
	struct packet *pkt = (struct packet*)args;
	DEBUG(INFO, "%s %d %d %u handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	pkt->data += sizeof(struct pma_pms_header);
	pkt->len -= sizeof(struct pma_pms_header);
	send_message_to_module(PEM, BGP_MRAI, pkt->data, pkt->len);
	int ret = send_ack_to_pms(pkt);
	if(ret)
		return 0;
	return -1;
}

int handle_ops_pms_neighbor_list(void *args)
{
	struct packet *pkt = (struct packet*)args;
	pkt->data += sizeof(struct pma_pms_header);
	pkt->len -=  sizeof(struct pma_pms_header);
	DEBUG(INFO, "%s %d %d %u handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	process_neighbor_list(pkt->data);
	int ret = send_ack_to_pms(pkt);
	if(ret)
		return 0;
	return -1;
}

int handle_trans_message(void *args)
{
	struct packet *pkt = (struct packet*)args;
	DEBUG(INFO, "%s %d %d %u handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	pkt->data += sizeof(struct common_header);
	pkt->len -= sizeof(struct common_header);
	char *buff = pkt->data;
	int len = pkt->len;
	int type = pkt->ops_type;

	if( buff == NULL || len == 0)
	{
		DEBUG( ERROR, "Trans message Empty");
		return ENORMERR;
	}
	char	*pms_addr;
	int	    pms_port = 0;
	int	    iret = 0;
	pms_addr = get_server_address();
	pms_port = get_server_port();
	if( pms_addr == NULL )
	{
		DEBUG( ERROR, "\n[%s][%d]pms_addr error\n", __FILE__, __LINE__ );
		return ENORMERR;
	}
	DEBUG( INFO, "\n[%s][%d]pms_addr=[%s] pms_port=[%d]\n", __FILE__, __LINE__, pms_addr, pms_port );

	iret = send_message_to_pms( pms_addr, pms_port, type, buff, len );
	if( iret != SUCCESS )
	{
		DEBUG( ERROR, "\n[%s][%d]send message to pms error\n", __FILE__, __LINE__ );
	}
	return SUCCESS;
}

int handle_ospf_spf_signal_ic_to_dbm(void *args)
{
	struct pkt * info = (struct pkt*)args;
	local_module_header *pkt = (local_module_header*)(info->data);
	char *buff = (char*)pkt->pkt;
	int len = pkt->pkt_len - sizeof(local_module_header);
	return send_message_to_module(DBM, OSPF_SPF, buff, len);
}

int handle_ops_pms_tc_cmd(void *args)
{
	struct packet *pkt = (struct packet*)args;
	DEBUG(INFO, "%s %d %d %u handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);

	int ret = send_ack_to_pms(pkt);
	if(ret)
		return 0;
	ret = create_connect(get_rltm_address(), get_rltm_port());
	if( ret == -1)
	{
		DEBUG(ERROR, "can't send the command to tc, create connection error");
		return -1;
	}
	struct packet send_pkt;
	int pkt_len = pkt->len - sizeof(struct pma_pms_header);
	pkt->data += sizeof(struct pma_pms_header);


	send_pkt.ops_type = TC_CMD_PMS_TO_PEA;
	send_pkt.len = pkt_len;
	send_pkt.sockfd = ret;

	send_pkt.timeout = 5;
	strncpy(send_pkt.ip,get_rltm_address(),sizeof(TC_CMD_ADDR));

	int errorcode = encapsulate_packet(&send_pkt,pkt->data);
	if (errorcode == -1){
		DEBUG(ERROR, "%s %d %d encapsulate_packet", pkt->ip, pkt->sockfd,pkt->ops_type);
		close_connect(ret);
		return -1;
	}

	errorcode = send_packet(&send_pkt);
	if (errorcode == -1){
		DEBUG(ERROR, "%s %d %d send_packet", pkt->ip, pkt->sockfd,pkt->ops_type);
		close_connect(ret);
		return -1;
	}

	errorcode = recv_packet(&send_pkt,1);//old packet format
	if (errorcode == -1){
		DEBUG(ERROR, "%s %d %d recv_packet", pkt->ip, pkt->sockfd,pkt->ops_type);
		close_connect(ret);
		return -1;
	}
	close_connect(ret);

	struct common_header* ptr = (struct common_header*)send_pkt.data;
	send_pkt.len -= sizeof(struct common_header);
	send_pkt.data = ptr + sizeof(struct common_header);
	DEBUG(INFO,"%d",send_pkt.len);

	//	ret = handle_ops_tc_cmd_ack_to_pms(&send_pkt);
	if(ret == 0)
	{
		DEBUG(INFO,"COMMAND TC EXCUATE SUCCESS");
		return 0;
	}
	else
	{
		DEBUG(ERROR,"COMMAND TC EXCUATE ERROR");
		return -1;
	}
}
int handle_trans_packet_to_pms(void *args)
{
	struct packet *pkt = (struct packet*)args;
	DEBUG(INFO, "%s %d %d %u handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	memcpy(pkt->ip , get_server_address(), sizeof(pkt->ip)-1);
	pkt->port = get_server_port();
	int ret = send_message_to_pms( pkt->ip , pkt->port , pkt->ops_type, pkt->data, pkt->len);
	return ret;
}

//OPS_PMS_LOGIN_RESP_OK
int handle_ops_pms_login_resp_ok(void *args)
{
	struct packet *pkt = (struct packet*)args;
	DEBUG(INFO, "%s %d %d %u handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
#ifndef OSPF_VERSION
	return 0;
#endif
	int ret = 0;
	ret = recv_packet(pkt, 0);
	if(ret == -1)
	{
		DEBUG(INFO, "%s %d %d %u handle_ops_pms_login_resp_ok handle :recv packet failed",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
		return -1;
	}
	DEBUG(INFO, "%s %d %d %u RECEIVE POLICY DATA",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	DEBUG(DATA, "%s", pkt->data + sizeof(struct pma_pms_header));
	int len  = pkt->len - sizeof(struct pma_pms_header);
	xml_policy_buff.buff = (char*)malloc(len);
	xml_policy_buff.length = len;
	memcpy(xml_policy_buff.buff, pkt->data + sizeof(struct pma_pms_header),xml_policy_buff.length);
    printf("%d %s\n",xml_policy_buff.length, xml_policy_buff.buff);

	/*Macro.Z 2013-03-27 end*/
//	if (process_policy(pkt->data + sizeof(struct pma_pms_header), pkt->len) == -1) {
//		DEBUG(ERROR, "%s %d %d %u POLICY PROCESS FAILED",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
//		return -1;
//	}
	DEBUG(INFO, "%s %d %d %u POLICY PROCESS SUCCEED",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	return 0;
}
//show packet content
int handle_packet_show_content(void *args)
{
	struct packet *pkt = (struct packet*)args;
	DEBUG(INFO, "%s %d %d %u handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	DEBUG(INFO,"%s",pkt->data+ sizeof( struct pma_pms_header));
}
//OPS_PMS_LOGIN_FAILED
int handle_ops_pms_login_failed(void *args)
{
}
//OPS_ACK_FAILED
int handle_ops_ack_failed(void *args)
{
}
//OPS_ACK
int handle_ops_ack(void *args)
{
}
int process_neighbor_list(char* xml){
	if(rList == NULL)
		init_router_list();

	int router_id;
	struct in_addr pma_addr;
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;

	doc = xmlParseDoc((xmlChar *)xml);
	if (doc == NULL) {
		DEBUG(ERROR, "xmlParseDoc failed %s", strerror(errno));
		return -1;
	}

	root_node = xmlDocGetRootElement(doc);
	if (root_node == NULL) {
		xmlFreeDoc(doc);
		DEBUG(ERROR, "xmlDocGetRootElement failed %s", strerror(errno));
		return -1;
	}

	xmlNodePtr cur_node = NULL;
	xmlNodePtr child_node = NULL;
	xmlChar * cur_data = NULL;
	char *endptr = NULL;
	int base = 10;
	int count = 0;
	for ( cur_node = root_node->children; cur_node != NULL; cur_node = cur_node->next)
	{
		if(xmlStrcmp(cur_node->name, (const xmlChar*)("timestamp")) == 0)
		{
			continue;
		}
		if(xmlStrcmp(cur_node->name, (const xmlChar*)("PMA"))==0)
		{
			count++;
			xmlChar* pma_id = xmlGetProp(cur_node, "id");
			router_id = strtol((char *)pma_id, &endptr, base);
			for(child_node = cur_node->children;child_node!=NULL;child_node = child_node->next){
				if (child_node->type == XML_ELEMENT_NODE) {
					if(xmlStrcmp(child_node->name, (const xmlChar* )("ipv4_address"))== 0){
						xmlChar* addr = xmlNodeListGetString(doc, child_node->xmlChildrenNode, 1);
						//printf("IP:%s\n",addr);
						inet_pton(AF_INET,addr,&pma_addr);
						pthread_mutex_lock(&router_list_mutex);
						router_id = htonl(router_id);
						insert_router_list(router_id,pma_addr.s_addr);
						pthread_mutex_unlock(&router_list_mutex);
						xmlFree(addr);
					}
				}
			}	
			xmlFree(pma_id);
		}
	}
	if (router_list_timer == -1)
	{
		router_list_timer = set_timer(5,send_router_list_to_ic,NULL,0);
	}
	else
	{
		update_timer(router_list_timer);
	}
	xmlFreeDoc(doc);
}


//MODREG
int handle_local_module_register(void *args)
{
	struct pkt* info = (struct pkt*)args;
	local_module_header * pkt = (local_module_header*)(info->data);
	int m_id = 0;
	int m_sockfd = info->fd;
	m_id = *((int*)pkt->pkt);
	DEBUG(INFO,"Module[%d] PkgType[%d] length [%d]",m_id, pkt->pkt_type, pkt->pkt_len);

	int already_registered = 0;
	already_registered = insert_register_table( m_id, m_sockfd, "" );
	int len = sizeof(local_module_header)+sizeof(struct response_message);
	pkt = (local_module_header*)malloc(len);
	if(pkt==NULL)
	{
		DEBUG(ERROR,"malloc error");
		return 0;
	}
	char routerid[24];
	sprintf(routerid,"%d",get_pma_id());
	pack_response_message( pkt,m_id, 0, routerid );

	int ret = send_pkt(m_sockfd,pkt,len);
	fprintf( stdout, "[%s][%d]send response to module [%d],len=[%d]\n", __FILE__, __LINE__, m_id, len );
	if( ret < 0 )
	{
		free(pkt);
		fprintf( stdout, "[%s][%d]write error=[%s]\n", __FILE__, __LINE__, strerror(errno) );
		close( m_sockfd );
		return -1;
	}
	//sleep(2);
	send_config_to_module(m_id);
	sleep(1);
	switch(m_id)
	{
		case ICM:
			{
				if(!already_registered)bm_status++;
				save_router_list_to_xml();
				if(global_pma_buff.length != 0)
				{
					//printf("%d %s \n",global_pma_buff.length,global_pma_buff.buff+sizeof(local_module_header));
					send_message_to_module(ICM, PMA_LIST, global_pma_buff.buff, global_pma_buff.length);
				}
				break;
			}
		case DBM:
			{
				if(!already_registered)bm_status++;
				send_message_to_module(DBM, POLICY_LIST, xml_policy_buff.buff, xml_policy_buff.length);
				break;
			}
		case PEM:
			{
				if(!already_registered)bm_status++;
				break;
			}
		default:
			break;
	}

	fprintf( stdout, "[%s][%d]handle local module register end\n", __FILE__, __LINE__ );
	free(pkt);

	return 0;
}
//LSDB_INFO
int handle_lsdb_snapshoot_dbm_to_pms(void *args)
{
	struct pkt * info = (struct pkt*)args;
	local_module_header *pkt = (local_module_header*)(info->data);
	char *buff = (char*)pkt->pkt;
	int len = pkt->pkt_len - sizeof(local_module_header);
	send_message_to_pms(get_server_address(), get_server_port(), OPS_PMA_SNAPSHOT_SEND, buff, len); 
}
//NETWORK_INFO
int handle_network_info_dbm_to_pms(void *args)
{
}
//ADDLSA
int handle_add_lsa_ic_to_dbm(void *args)
{
	struct pkt* info = (struct pkt*)args;
	local_module_header * pkt = (local_module_header*)(info->data);
	char* buff;
	int m_sockfd = info->fd;
	int info_type = pkt->pkt_type;
	int len = pkt->pkt_len - sizeof(local_module_header);
	buff = (char*)pkt->pkt;
	DEBUG(INFO,"PkgType[%d] length [%d]", pkt->pkt_type, pkt->pkt_len);
	return send_message_to_module(DBM, info_type,buff, len);  
}
//UPDATELSA.
int handle_update_lsa_ic_to_dbm(void *args)
{
	struct pkt* info = (struct pkt*)args;
	local_module_header * pkt = (local_module_header*)(info->data);
	char* buff;
	int m_sockfd = info->fd;
	int info_type = pkt->pkt_type;
	int len = pkt->pkt_len - sizeof(local_module_header);
	buff = (char*)pkt->pkt;
	DEBUG(INFO,"PkgType[%d] length [%d]", pkt->pkt_type, pkt->pkt_len);
	return send_message_to_module(DBM, info_type,buff, len);  
}
//ADDROUTE
int handle_add_route_ic_to_pms(void *args)
{
}
//POLICY_INFO
int handle_send_policy_dbm_to_pea(void *args)
{
	struct pkt* info = (struct pkt*)args;
	local_module_header * pkt = (local_module_header*)(info->data);
	char* buff;
	int m_sockfd = info->fd;
	int info_type = pkt->pkt_type;
	int len = pkt->pkt_len - sizeof(local_module_header);
	buff = (char*)pkt->pkt;
	DEBUG(INFO,"PkgType[%d] length [%d]", pkt->pkt_type, pkt->pkt_len);
	return send_message_to_module(PEM, info_type,buff, len);  
}

char* save_router_table_to_xml(char* rtable)
{
	xmlDocPtr doc = create_xml_doc();
	if (rtable == NULL)
		return NULL;
	xmlNodePtr routernode;
	xmlNodePtr childnode;

	xmlNodePtr rootnode = create_xml_root_node(doc, "SNAPSHOT");
	
	routernode = add_xml_child(rootnode, "ROUTER", NULL);
	char* rid[24];
	sprintf(rid,"%d", get_pma_id());
	add_xml_child_prop(routernode, "id",rid); 
	
	struct timeval now;
	gettimeofday(&now, NULL);
	char * time = PRINTTIME(now);
	add_xml_child(routernode, "timestamp",time); free(time);
	
	add_xml_child(routernode, "LSDB", NULL);

	childnode = add_xml_child(routernode, "Routing_table", NULL);
	
	add_xml_child(childnode,"IPV4",rtable);

	u8 *xmlbuff;
	int len  = 0;
	xmlDocDumpFormatMemoryEnc( doc, &xmlbuff, &len, "UTF-8", 0 );
	char *buff = (char*)malloc(len+1);
	memcpy(buff, xmlbuff, len);
	buff[len]= 0;
	xmlFree(xmlbuff);
	xmlFreeDoc(doc);
	return buff;

}

//UP_ROUTE_INFO
int handle_route_table_ic_to_pms(void *args)
{
	struct pkt* info = (struct pkt*)args;
	local_module_header *pkt = (local_module_header*)(info->data);
	int pkt_len = pkt->pkt_len - sizeof(local_module_header);
	char * data = (char*)malloc(pkt_len+1);
	memcpy(data, pkt->pkt, pkt_len);
	data[pkt_len] = 0;
	char *buff = save_router_table_to_xml(data);
	int len = strlen(buff);
	return send_message_to_pms(get_server_address(), get_server_port(),
		OPS_PMA_SNAPSHOT_SEND, buff, len);
	
}
//UP_INTERFACE_INFO
int handle_tc_message_to_pms(void *args)
{
	struct pkt* info = (struct pkt*)args;
	local_module_header *pkt = (local_module_header*)(info->data);
	char *buff = (char*)pkt->pkt;
	int info_type = pkt->pkt_type;
	int len = pkt->pkt_len - sizeof(local_module_header);
	return send_message_to_pms(get_server_address(), get_server_port(), 
			TC_TO_PMA_INTERFACE_INFO, buff, len);
}

int send_router_list_to_ic()
{
	int ret = save_router_list_to_xml();
	if(ret == -1) return -1;
	send_message_to_module(ICM,PMA_LIST, global_pma_buff.buff, global_pma_buff.length);
	printf("Send Router List to IC\n");
	unset_timer(router_list_timer);
	router_list_timer = -1;
	pthread_detach(pthread_self());
}

int save_router_list_to_xml()
{
	if (rList == NULL) return -1;
	/*	local_module_header* n_pkt= (local_module_header*)malloc(1024);
		n_pkt->pkt_version = PMA_VERSION;
		n_pkt->pkt_type = PMA_LIST;
		n_pkt->checksum = 12345;
		char* pkt_ptr = ;
		char* pkt_ptr = ;
		*/	
	char* pkt_ptr = (char*)malloc(1024);
	char* ptr = pkt_ptr;
	struct router_list* tmp = rList->next;
	int count = 0;
	while(tmp!=NULL)
	{
		memcpy(pkt_ptr,&tmp->rid,sizeof(int));
		printf("RouterID:%d\n",tmp->rid);
		memcpy(pkt_ptr+sizeof(int),&tmp->ip_addr,sizeof(int));
		printf("RouterIP:%d\n",tmp->ip_addr);
		pkt_ptr = pkt_ptr+2*sizeof(int);	
		count++;
		tmp = tmp->next;
	}
	int len = count*2*sizeof(int);
	printf("########%d##########\n",len);
	memcpy(global_pma_buff.buff,ptr,len);
	global_pma_buff.length = len;
	free(ptr);
	return 0;
}

