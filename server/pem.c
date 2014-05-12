#include <utils/common.h>
#include <utils/utils.h>
#include <socket.h>
#include <server.h>
#include <srvconf.h>
#include <control/control.h>
#include <logger/logger.h>
#include <snmp/includes/pma_api.h>
#include "pem.h"

ifTable *ifList;
int ifaceList[MAX_INTERFACE];

struct pem_conf conf;

int pem_init()
{
	log_type_init();
	log_init();
	module_init(PEM);
	set_conf_type(PEA_CONF);
	add_ctl(packet_handler, PEA_CONF, conf_handle, 1 );
	add_ctl(packet_handler, ACK, ack_handle, 1);
	add_ctl(packet_handler, POLICY_INFO,policy_handle,1 );
} 

int pem_start()
{
	logger("PMA_REGISTER","pma module pem started",1);
	module_start();
}

int conf_handle(struct Packet_header* pkt)
{
	if( pkt->pkt_len == 0)return 0;
	conf = *(struct pem_conf*)(pkt->pkt);
	char tmp[25];
	inet_ntop(AF_INET,&conf.router_ip, tmp, 24);
	DEBUG(INFO, "Config parse: %d %s\n",conf.pma_id, tmp);
	init_interface_list();
	return 1;
}

int policy_handle(struct Packet_header* pkt)
{
	if( pkt->pkt_len == 0) return 0;
	policyMsgHeader* msgHeader = (policyMsgHeader*)(pkt->pkt);
	if(msgHeader->operation == 0&&msgHeader->pepversion == 3)
	{
		policyMsg* msg = msgHeader->msg;
		short hello_interval = msg->hello_interval/TIMEPART;
		short dead_interval = msg->dead_interval/TIMEPART;
		short restransmit_interval = msg->restransmit_interval/TIMEPART;

		DEBUG(INFO,"%d\n",hello_interval);
		DEBUG(INFO,"%d\n",dead_interval);
		DEBUG(INFO,"%d\n",restransmit_interval);
		DEBUG(INFO,"%d\n",msg->if_id);
		if(hello_interval == 0 )hello_interval = 1;
		if(dead_interval == 0) dead_interval = 1;
		//printf("%d\n",hello_interval);
		//printf("%d\n",dead_interval);
		//printf("%d\n",restransmit_interval);
		//printf("%d\n",msg->if_id);
		if(msg->if_id <= 0)return 0;
		if(ifaceList[msg->if_id]==-1)return 0;
		char router_ip[24];
		char interface_id[15];
		char hello[50],dead[50],restransmit[50];

		sprintf(hello,"%d",hello_interval);
		sprintf(dead,"%d\n",dead_interval);
		sprintf(restransmit,"%d\n",restransmit_interval);
		inet_ntop(AF_INET,&conf.router_ip,router_ip,24);
		sprintf(interface_id,"%d",ifaceList[msg->if_id]);
		int ret1 = setHelloInteval(router_ip,interface_id,hello);
		int ret2 = setDeadInteval(router_ip,interface_id,dead);
		//setRetransmitInteval(router_ip,interface_id,restransmit);
	}
	else
		DEBUG(INFO,"Not the Right Policy");

}
int init_interface_list()
{
	memset(ifaceList,0xFF,sizeof(int)*MAX_INTERFACE);
	char router_ip[24];
	inet_ntop(AF_INET,&conf.router_ip,router_ip,24);
	ifList = NULL;
	getIfInfos(router_ip,&ifList);
	if(ifList == NULL)
	{
		DEBUG(ERROR,"Get Interface List failed");
		exit(-1);
	}
	ifTable* tmp = ifList->next;
	int i=0,j = 0;
	while(tmp)
	{
		i = atoi(tmp->ifdex);
		j = atoi(tmp->ifDescrDex);
		if(i>=0)
			ifaceList[i]= j;
		tmp = tmp->next;
	}
	DEBUG(INFO,"Get interface List success");
};
