#include <utils/common.h>
#include <utils/utils.h>
#include <utils/xml.h>
#include <server/pem.h>
#include <mpls.h>
#include "routercmd.h"

extern struct pem_conf conf;
/**
 * 执行cmd命令
 */
int runRouterCmd(struct tunnelcmd *cmd) {

	if ( is_fastmpls() )
	{
		__u32 rkey;
		__u32 key;
		__u32 num;
		int type = cmd->type;
		switch( type ){
			case 0x01:
				add_mpls_in(cmd->out_label, cmd->out_if, cmd->next_ip ,
					&rkey);
				get_iptables( strcmp( cmd->src_ip, "") ? cmd->src_ip : NULL, 
						cmd->dst_ip, &num, &key);
				if( num != 0){
					del_iptables(num);
				}
				add_iptables( strcmp(cmd->src_ip, "") ? cmd->src_ip : NULL, 
						cmd->dst_ip, rkey);
				break;
			case 0x02:
				add_mpls_end(cmd->in_label);
				add_mpls_in(cmd->out_label, cmd->out_if, cmd->next_ip,
						&rkey);
				add_mpls_mid(cmd->in_label, rkey);
				break;
			case 0x03:
				add_mpls_end(cmd->in_label);
				break;
			case 0x04:

			case 0x05:
			default:break;;
		}

	}
	else{//System()
		char command[1024];
		memset(command, 0x0, 1024);
		int len = 0;
		sprintf(command, ROUTER_CMD_PATH);
		len = strlen(command);
		if( conf.device_type/10 == 1 )//Quagga
		{
			sprintf(command+len, "%02x ", cmd->type);
		}
		else if ( conf.device_type/10 == 2) //Huawei
		{
			sprintf(command+len, "%02x.tcl ", cmd->type);
		}
		else {
			sprintf(command+len, "%02x ", cmd->type);
		}
		len = strlen(command);

		if( strcmp(cmd->src_ip,"") != 0 ) 
		{
			sprintf(command+len, "%s ", cmd->src_ip);
			len = strlen(command);
		}

		if( strcmp(cmd->dst_ip,"") != 0 ) 
		{
			sprintf(command+len, "%s ", cmd->src_ip);
			len = strlen(command);
		}

		if( cmd->out_label != 0 ) 
		{
			sprintf(command+len, "%d ", cmd->out_label);
			len = strlen(command);
		}

		if( cmd->in_label != 0 ) 
		{
			sprintf(command+len, "%d ", cmd->in_label);
			len = strlen(command);
		}

		if( strcmp(cmd->out_if,"") != 0 ) 
		{
			sprintf(command+len, "%s ", cmd->out_if);
			len = strlen(command);
		}

		if( strcmp(cmd->next_ip,"") != 0 ) 
		{
			sprintf(command+len, "%s ", cmd->next_ip);
			len = strlen(command);
		}

		if( cmd->outport!= 0 ) 
		{
			sprintf(command+len, "%d ", cmd->outport);
			len = strlen(command);
		}
		printf("%s\n", command);
		int ret = system(command);
		printf("cmd seq = %d\n", ret);
		if( ret !=0 ){
			DEBUG(ERROR, "cmd run error");
			return -1;
		}
	}
	return 0;
}

int ExecuteRouterCMD(const char* xml, int len)
{
	if (xml == NULL || len <= 0) {
		return -1;
	}
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlNodePtr contextnode;
	xmlNodePtr flownode;
	DEBUG(INFO,"recive cmd");

	doc = xmlParseMemory(xml, len);
	if (doc == NULL) {
		perror("config doc is null!");
		return (-1);
	}
	
	struct tunnelcmd command;
	memset(&command, 0x0, sizeof(command));

	node = xmlDocGetRootElement(doc);

	xmlChar* cmd_type = get_value_by_name(doc, node, "type");
	command.type = adv_atoi(cmd_type,16);
	printf("%s %d\n", cmd_type, command.type);
	xmlFree(cmd_type);

	contextnode = get_node_by_name(doc, node , "content");

	flownode = get_node_by_name(doc, contextnode, "flow");

	xmlChar* cmd_src_ip = get_value_by_name(doc, flownode, "src_ip");
	xmlChar* cmd_dst_ip = get_value_by_name(doc, flownode, "dst_ip");
	xmlChar* cmd_out_label = get_value_by_name(doc, flownode, "out_label");
	xmlChar* cmd_in_label = get_value_by_name(doc, flownode, "in_label");
	xmlChar* cmd_out_if = get_value_by_name(doc, flownode, "out_if");
	xmlChar* cmd_next_ip = get_value_by_name(doc, flownode, "next_ip");
	xmlChar* cmd_out_port = get_value_by_name(doc, flownode, "out_port");

	if( cmd_src_ip != NULL)
	sprintf(command.src_ip, "%s", cmd_src_ip);		
	if( cmd_dst_ip != NULL)
	sprintf(command.dst_ip, "%s", cmd_dst_ip);		
	if( cmd_out_label != NULL)
	command.out_label = adv_atoi(cmd_out_label, 10);
	if( cmd_in_label != NULL)
	command.in_label = adv_atoi(cmd_in_label, 10);
	if( cmd_out_if != NULL)
	sprintf(command.out_if, "%s", cmd_out_if);
	if( cmd_next_ip != NULL)
	sprintf(command.next_ip, "%s", cmd_next_ip);
	if( cmd_out_port != NULL)
	command.outport = adv_atoi(cmd_out_port,10);

	xmlFree(cmd_src_ip);
	xmlFree(cmd_dst_ip);
	xmlFree(cmd_out_label);
	xmlFree(cmd_in_label);
	xmlFree(cmd_out_if);
	xmlFree(cmd_next_ip);
	xmlFree(cmd_out_port);

	DEBUG(INFO,"run cmd");
	runRouterCmd((struct tunnelcmd*)&command);
	DEBUG(INFO,"end cmd");

	xmlFreeDoc(doc);
	return 0;
}
