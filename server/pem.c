#include <utils/common.h>
#include <utils/utils.h>
#include <socket.h>
#include <server.h>
#include <srvconf.h>
#include <control/control.h>
#include <logger/logger.h>
#include <snmp/includes/pma_api.h>
#include <utils/xml.h>
#include <command/routercmd.h>
#include "pem.h"

#define CMD_FILE	"./command/xml-cmd/"

ifTable *ifList;
int ifaceList[MAX_INTERFACE];
struct bgp_interface * ebgp_list;
struct bgp_interface * ibgp_list;
char router_ip[24];
char as_id[24];

struct pem_conf conf;


int  read_cmd_from_file(int no);
int is_fastmpls()
{
	return conf.fast_mpls;
}

int pem_init()
{
//	read_cmd_from_file(2);
	log_type_init();
	log_init();
	module_init(PEM);
	set_conf_type(PEM_CONF);
	add_ctl(packet_handler, PEM_CONF, conf_handle, 1 );
	add_ctl(packet_handler, ACK, ack_handle, 1);
	add_ctl(packet_handler, POLICY_INFO,policy_handle,1 );
	add_ctl(packet_handler, BGP_MRAI, mrai_handle, 1);
	add_ctl(packet_handler, TC_TUNNEL_COMMAND, tunnel_command_handle, 1);
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
	if ( conf.device_type%10 == 2 )//bgp
	init_bgp_interface();
	return 1;
}
//Get the interface running egp
int init_bgp_interface()
{
    ebgp_list = (struct bgp_interface *)malloc(sizeof(struct bgp_interface));
    ibgp_list = (struct bgp_interface *)malloc(sizeof(struct bgp_interface));
    ebgp_list->next = NULL;
    ibgp_list->next = NULL;
    inet_ntop(AF_INET,&conf.router_ip,router_ip,24);
    ifTable *iflist = NULL;
    ifTable *ifp = NULL;
    getBgpIfInfos(router_ip,&iflist);
    if(iflist == NULL)
    {
        printf("Get interface info from snmp error\n");
        return -1;
    }
    struct bgp_interface *handle = ebgp_list;
    struct bgp_interface *handle_i = ibgp_list;
    getBgpLocalAs(router_ip,as_id);
    int localasid = atoi(as_id);
    ifp = iflist->next;
    while(ifp!=NULL)
    {
        int remoteasid = atoi(ifp->areaId);
        if(remoteasid == localasid)//ibgp
        {
            handle_i->next = (struct bgp_interface *)malloc(sizeof(struct bgp_interface));
            handle_i = handle_i->next;
            handle_i->ifid = atoi(ifp->ifdex);
            strcpy(handle_i->name, ifp->ifDescr);
            strcpy(handle_i->local_ip, ifp->ip);
            strcpy(handle_i->neighbor_ip, ifp->ospfNbrIp);
            handle_i->next = NULL;
            //handle_i = handle_i->next;
        }
        else if(remoteasid != 0)//ebgp
        {
            handle->next = (struct bgp_interface *)malloc(sizeof(struct bgp_interface));
            handle = handle->next;
            handle->ifid = atoi(ifp->ifdex);
            strcpy(handle->name, ifp->ifDescr);
            strcpy(handle->local_ip,ifp->ip);
            strcpy(handle->neighbor_ip,ifp->ospfNbrIp);
            handle->next = NULL;
        }
        ifp = ifp->next;
    }
    handle = ebgp_list->next;
    while(handle!=NULL)
    {
        printf("Inteface %d\n",handle->ifid);
        printf("Name %s\n",handle->name);
        printf("LocalAddress %s\n",handle->local_ip);
        printf("RemoteAddress %s\n",handle->neighbor_ip);
        handle = handle -> next;
    }
    handle_i = ibgp_list->next;
    while(handle_i!=NULL)
    {
        printf("Inteface %d\n",handle_i->ifid);
        printf("Name %s\n",handle_i->name);
        printf("LocalAddress %s\n",handle_i->local_ip);
        printf("RemoteAddress %s\n",handle_i->neighbor_ip);
        handle_i = handle_i -> next;
    }
}


int set_interface_mrai(char* ifname, int value)
{
    if(value < 0 || value > 100)
    {
        printf("Mrai value not right\n");
        return -1;
    }

    struct bgp_interface *handle = ebgp_list->next;
    while(handle!=NULL)
    {
        if(strcmp(handle->name, ifname) == 0)
        {
            DEBUG(INFO,"RouterIP:%s Area: %s Neighbor: %s Value:%d",router_ip, as_id ,handle->neighbor_ip, value);
            setBgpPeerUpdateInterval(router_ip, as_id, handle->neighbor_ip, value);
            break;
        }
        handle = handle -> next;
    }
    return 0;
}
int handle_mrai_value(char* pkt, int length)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;

    char* xmlbuff = (char*)malloc(length);
    memcpy(xmlbuff, pkt, length);

    doc = xmlParseDoc((xmlChar *)pkt);

    if(doc == NULL)
    {
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
        char *endptr = NULL;
        int base = 10;
        int count = 0;
    xmlChar* name;
    xmlChar* ifname;
    for (cur_node = root_node->children; cur_node!= NULL; cur_node = cur_node->next) {
                if (cur_node->type == XML_ELEMENT_NODE) {
                        if (xmlStrcmp(cur_node->name, (const xmlChar *)("timestamp")) == 0) {
                                continue;
                                }
            if (xmlStrcmp(cur_node->name, (const xmlChar *)("item")) == 0 )
            {
               name =  xmlGetProp(cur_node, "name");
               if(xmlStrcmp(name , (const xmlChar*)("interfacename")) == 0)
               {
                    ifname = xmlNodeListGetString(doc, cur_node->xmlChildrenNode, 1);

               }
               else if(xmlStrcmp(name , (const xmlChar*)("mrai")) == 0)
               {
                    xmlChar* Value = xmlNodeListGetString(doc,cur_node->xmlChildrenNode,1);
                    int value = strtol((char *)Value, &endptr, base);
                    set_interface_mrai((char*)ifname,value);
                    xmlFree(name);
                    xmlFree(Value);
               }
            }
                }
    }


    //From PMS recevied a mrai value, then we set it to the interface running EGP
    //first get the value from packet
   /* char sdata[64];
    strncpy(sdata,pkt,length);
    int value = atoi(sdata);
    */
    return 0;


}
int mrai_handle(struct Packet_header* pkt)
{
	if( pkt->pkt_len == 0) return 0;
	int ret = handle_mrai_value(pkt->pkt, pkt->pkt_len);
	return ret;
}
int tunnel_command_handle(struct Packet_header* pkt)
{
	if( pkt->pkt_len == 0) return 0 ;
	int ret = ExecuteRouterCMD(pkt->pkt, pkt->pkt_len);
	return ret;
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

int  read_cmd_from_file(int no)
{
	char filepath[100];
	sprintf(filepath, CMD_FILE);
	int k = strlen(filepath);
	sprintf(filepath+k, "%02d.xml", no);
	printf("%s\n" , filepath);

	FILE*fp = fopen(filepath,"r");
	int len = 0;
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	printf("%d\n", len);
	fseek(fp, 0, SEEK_SET);
	char* xmlbuff = (char*)malloc(len+1);
	fread(xmlbuff, 1, len, fp);
	printf("%s\n", xmlbuff);
	int ret = ExecuteRouterCMD( xmlbuff, len);
	free(xmlbuff);
	return ret;
}
