#include <utils/common.h>
#include <utils/utils.h>
#include <comm/header.h>
#include <comm/comm.h>
#include <conf/conf.h>
#include <utils/xml.h>
#include <reg.h>

u8 agent_login_status = AGENT_LOGOUT;

static void *login(void *args);

int agent_init_request(char* routerid)
{
	//pack the login xml
	xmlDocPtr doc = create_xml_doc();
	xmlNodePtr devnode;
	xmlNodePtr childnode;

	xmlNodePtr rootnode = create_xml_root_node(doc,"AGENT_INIT");
	struct timeval now;
	gettimeofday(&now, NULL);
	char * time = PRINTTIME(now);
	add_xml_child(rootnode, "timestamp",time); free(time);

	devnode = add_xml_child(rootnode, "device", NULL);
	char aid[24];
	sprintf(aid,"%d", get_pma_id());
	add_xml_child_prop(devnode, "id",aid); 
	char dev_type[10];
	if( get_protocol_type() == OSPF_ROUTER )
	{
		sprintf(dev_type, "ospf");
		add_xml_child(devnode, "type", dev_type);
	}
	else if( get_protocol_type() == BGP_ROUTER )
	{
		sprintf(dev_type, "bgp");
		add_xml_child(devnode, "type", dev_type);
	}
	else {}

	char asid[24];
	inet_ntop(AF_INET, &pma_conf.as_num , asid, 24);
	add_xml_child(devnode, "as_id", asid);
	add_xml_child(devnode, "router_id", routerid);
	char ctlip[24];
	inet_ntop(AF_INET, &pma_conf.ic_config.local_ip, ctlip,24);
	add_xml_child(devnode, "agent_control_ip", ctlip);
	
	char* xmlbuff;
	int len = 0;
	xmlDocDumpFormatMemoryEnc( doc, &xmlbuff, &len, "UTF-8", 0);
	char* buff = (char*)malloc(len+1);
	memcpy(buff, xmlbuff, len);
	buff[len] = 0;
	xmlFree(xmlbuff);
	xmlFreeDoc(doc);

	len = strlen(buff);
	char rid[24];
	strcpy(rid,routerid);
	struct req_args *req_arg =
		(struct req_args *)malloc(sizeof(struct req_args));

	memset(req_arg->ip, 0, sizeof(req_arg->ip));
	char *ip = get_server_address();
	memcpy(req_arg->ip, ip, strlen(ip));
	req_arg->port = get_server_port();
	req_arg->ops_type = OPS_PMA_INIT_REQUEST;
	req_arg->len = 0;
	int ret = send_message_to_pms(req_arg->ip,
			req_arg->port,
			req_arg->ops_type,
			buff,
			len);
	free(req_arg);
	free(buff);
	return ret;
	 
}

int agent_register(u32 agent_id, const char *seq)
{

	//pack the login xml
	xmlDocPtr doc = create_xml_doc();
	xmlNodePtr devnode;
	xmlNodePtr childnode;

	xmlNodePtr rootnode = create_xml_root_node(doc,"AGENT_REGISTER");
	struct timeval now;
	gettimeofday(&now, NULL);
	char * time = PRINTTIME(now);
	add_xml_child(rootnode, "timestamp",time); free(time);

	devnode = add_xml_child(rootnode, "device", NULL);
	char aid[24];
	sprintf(aid,"%d", get_pma_id());
	add_xml_child_prop(devnode, "id",aid); 
	char dev_type[10];
	if( get_protocol_type() == OSPF_ROUTER )
		sprintf(dev_type, "ospf");
	else if( get_protocol_type() == BGP_ROUTER )
		sprintf(dev_type, "bgp");
	else {}
	add_xml_child(devnode, "type", dev_type);
	
	char* xmlbuff;
	int len = 0;
	xmlDocDumpFormatMemoryEnc( doc, &xmlbuff, &len, "UTF-8", 0);
	char* buff = (char*)malloc(len+1);
	memcpy(buff, xmlbuff, len);
	buff[len] = 0;
	xmlFree(xmlbuff);
	xmlFreeDoc(doc);

	struct req_args *login_args =
		(struct req_args *)malloc(sizeof(struct req_args)+len);

	memset(login_args->ip, 0, sizeof(login_args->ip));
	char *ip = get_server_address();
	memcpy(login_args->ip, ip, strlen(ip));
	login_args->port = get_server_port();
	login_args->ops_type = OPS_PMA_LOGIN;
	login_args->len = len;
	memcpy(login_args->data, buff, len);
	free(buff);
	if (login(login_args) == ((void *)-1)) {
		free(login_args);
		return -1;
	}
	free(login_args);
	agent_login_status = AGENT_LOGIN;
	return SUCCESS;
}

static void * login(void *args)
{
	struct req_args *login_args =
		(struct req_args *)args;

	int ret = send_message_to_pms(login_args->ip,
			login_args->port,
			login_args->ops_type,
			login_args->data,
			login_args->len);
	if (ret == -1) {
		return PTR_ERR;
	} else {
		return NULL;
	}
}

int agent_unregister(u32 agent_id, const char *seq)
{
	return SUCCESS;
}


int heart_beat_send()
{
	DEBUG(INFO,"Heart Beat Sended");
	int ret = send_message_to_pms( get_server_address(),  
			get_server_port(), OPS_PMA_HEARTBEAT,
			NULL, 0);
	pthread_detach(pthread_self());
	return ret;
}
