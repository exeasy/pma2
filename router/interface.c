#include <utils/common.h>
#include <utils/utils.h>
#include <utils/xml.h>
#include <lib/if.h>
#include <lib/prefix.h>
#include <lib/linklist.h>
#include <snmp/includes/pma_api.h>
#include <snmp/includes/snmpcore.h>
#include <server/srvconf.h>
#include "router.h"
#include "interface.h"

extern struct router localrouter;
struct ifTable* ifList;

//extern function
char* ospf_interface_info_to_xml(struct list* iflist);
char* device_info_to_xml(struct list* iflist);
char* ifrate_info_to_xml(struct list* iflist);
char* bgp_path_attr_to_xml(char* data);

struct list* iflist_init() {
    struct list * iflist = list_new();
    return iflist;
}

void interface_add(struct list * iflist, struct rinterface* iface)
{
    if(ifList == NULL) return;
    listnode_add(iflist, (void*)iface);
}

void interface_delete(struct list *iflist, struct rinterface *iface)
{
    if (ifList == NULL) return ;
    listnode_delete(ifList, (void*)iface);
}

int get_address_of_interface(struct rinterface* iface, struct list **result, int *total)
{
	if( iface == NULL ) return -1;
	if( result == NULL || *result == NULL ) return -1;
	if( total == NULL ) return -1;
	*total = 0;
	struct list* addr_list  = iface->details->connected;
	void * addr;
	struct listnode * addr_node;
	LIST_LOOP( addr_list, addr, addr_node )
	{
		struct connected * con_ip = (struct connected *) addr;
		listnode_add(*result, addr); 
		(*total)++;
	}
	return 0;
}

int get_interface(struct list * iflist, int qtype, void *query, struct list **result, int *total)
{
    if( iflist == NULL) return -1;
    if( query == NULL) return -1;
    if ( result == NULL || *result == NULL) return -1;
    if ( total == NULL ) return -1;
	*total = 0;
    struct rinterface *face;
    void * data;
    struct listnode * node;

    *total = 0;
/*;*/
    switch ( qtype ) {
        case Q_TYPE :
            LIST_LOOP(iflist, data, node)
            {
                face = (struct rinterface*)data;
                if(face->type == *(int*)query)
				{
                    listnode_add(*result, (void*)face);
					(*total)++;
				}
            }
            break;
        case Q_STATE :
            LIST_LOOP(iflist, data, node)
            {
                face = (struct rinterface*)data;
                if (face->op_state == *(int*)query)
				{
                    listnode_add(*result, (void*)face);
					(*total)++;
				}
            }
            break;
        case Q_NAME :
            LIST_LOOP(iflist ,data, node)
            {
                face = (struct rinterface*)data;
				char* p = (char*)query;
                if (strcmp(face->name, p) == 0)
				{
                    listnode_add(*result, (void*)face);
					(*total)++;
				}
            }
            break;        
        case Q_CTID :
            LIST_LOOP(iflist, data, node)
            {
                face = (struct rinterface *)data;
                if (face->ctl_id == *(int*)query)
				{
                    listnode_add(*result, (void*)face);
					(*total)++;
				}
            }
            break;
        case Q_NID:
            LIST_LOOP(iflist, data, node)
            {
                face = (struct rinterface *)data;
                if (face->neighbor->rid == *(int*)query)
				{
                    listnode_add(*result, (void*)face);
					(*total)++;
				}
            }
            break;
		case Q_IPADDR:
            LIST_LOOP(iflist, data, node)
            {
                face = (struct rinterface *)data;
				struct list * addrlist = face->details->connected;
				struct listnode * addrnode;
				void * addr;
				LIST_LOOP( addrlist, addr, addrnode)
				{
					struct connected * con_ip = (struct connected *)addr;
					if (con_ip->address->u.prefix4.s_addr == ((struct in_addr*)query)->s_addr )
					{
						listnode_add(*result, (void*)face);
						(*total)++;
					}
				}
            }
            break;

      //  case Q_RID :
      //      LIST_LOOP(iflist, data, node)
      //      {
      //          face = (struct rinterface *)data;
      //          if ( getrouterid() == *(int*)data)
      //              listnode_add(*result, (void*)face):
      //      }
      //      break;
    }
}

int load_interface_type_from_config()
{
	FILE* fp = fopen(CONFIGFILE,"r");
	if(fp== NULL){
		printf("Open config file failed\n");
		return 0;
	}
	char comm[100];
	char iface_name[IFNAMSIZ];
	enum if_type type  = ACCESS;
	int len;
	while(!feof(fp))
	{
		memset(comm,0,100);
		memset(iface_name,0, IFNAMSIZ);
		fscanf(fp,"%s",comm);
		printf("%s\n",comm);
		if(comm[0]=='#')continue;//or do sth else.
		if(0==strncmp(comm,"backbone",8)){
			fscanf(fp,"%s",iface_name);
			type =  BACKBONE;
			printf("%s %d\n",iface_name, 1);
		}
		else if(0==strncmp(comm,"access",6)){
			fscanf(fp,"%s",iface_name);
			type = ACCESS;
			printf("%s %d\n",iface_name,0);
		}
        else 
        {
            type = ACCESS;
        }
       struct list* iflist = localrouter.iflist;
       struct list* result = list_new();
       int total = 0;
       get_interface( iflist, Q_NAME, iface_name, &result , &total );
       if ( total >= 1)
       {
           struct rinterface* iface = (struct rinterface*)result->head->data;
           iface->type = type;;
		   printf("%s %d\n",iface->name, iface->type);
       }
	   list_delete_all_node(result);
	   list_free(result);
	}
	fclose(fp);
}

int check_interface_status(char* cstatus){
	int status = strcmp(cstatus,"down");
	if(status)return 1;
	else return 0;
}
int get_prefix_len(char* mask){
	if(*mask=='\0')return 0;
	struct in_addr n_mask;
	inet_aton(mask,&n_mask);
	u32 b_mask = (u32)n_mask.s_addr;
	b_mask = ~b_mask;
	int count = 0;
	while(b_mask){
		b_mask = b_mask<<1;
		count++;
		}
	return 32-count;
}

int set_neighbor_agent_address(int devid, int rid, int addr)
{
    struct list* iflist = localrouter.iflist;
    struct list* result = list_new();
    int total = 0;
    int qid = rid;
    char routerip[24];
    char destip[24];
    char gateway[24];
    inet_ntop(AF_INET, &localrouter.routerip, routerip, 24);
    inet_ntop(AF_INET, &addr, destip, 24);
    get_interface( iflist, Q_NID, &qid, &result , &total );
    if ( total >= 1)
    {
        struct rinterface* iface = (struct rinterface*)result->head->data;
        iface->neighbor->agent_addr.s_addr = addr;
		iface->neighbor->devid = devid; 
        inet_ntop(AF_INET,&iface->neighbor->router_addr, gateway,24); 
        addStaticRoute(routerip, destip, "255.255.255.255", gateway);
    }
	list_delete_all_node(result);
	list_free(result);
    return 0;
}

int ifrate_first_run = 0;
int  rate_refresh_timeval = 2;
int ifrate_diff_and_send(struct InterfaceTable* table)
{
	struct list* iflist = localrouter.iflist;
	if( iflist == NULL ){
		DEBUG(ERROR,"please run update_interface_from_snmp first");
		return -1;
	}
	struct InterfaceTable* ifnode = table->next;
	while(ifnode)
	{
		int i = atoi(ifnode->ifindex);
        struct list* result = list_new();
		struct rinterface* iface;

        int total = 0;
        get_interface( iflist, Q_CTID, &i, &result , &total );
        if ( total >= 1)
        {
            iface = (struct rinterface*)result->head->data;
			u64 send_first = iface->send_packet;
			u64 recv_first = iface->recv_packet;
			u64 send_second = atoll(ifnode->ifoutoctets);
			u64 recv_second = atoll(ifnode->ifinoctets);
			iface->send_rate = (send_second - send_first)/rate_refresh_timeval;
			iface->recv_rate = (recv_second - recv_first)/rate_refresh_timeval;
			iface->send_packet = send_second;
			iface->recv_packet = recv_second;
        }
		else{//find new interface
			
		}
	    list_delete_all_node(result);
	    list_free(result);
		ifnode = ifnode->next;
	}
	if( ifrate_first_run ==0 ){
		ifrate_first_run = 1;
		return 0;
	}
	char* buff = ifrate_info_to_xml(iflist);
	//send the packet
	int len = strlen(buff);
	module_send_data(buff, len , UP_TRAFFICE_INFO);
	free(buff);
}

int ifrate_detect(void *args,int type)
{
	if( type == 1 )
	get_if_table(ifrate_diff_and_send);
}


int bgp_path_attr_table_last_len = 0;
int bgppath_diff_and_send(struct bgpPathAttrTable* table)
{
	int is_changed = 0;
	struct bgpPathAttrTable* attrnode = table->next;
	char buff[10240];
	int len = 0;
	sprintf(buff,"bgp4PathAttrPeer bgp4PathAttrIpAddrPrefixLen bgp4PathAttrIpAddrPrefix bgp4PathAttrOrigin\
bgp4PathAttrASPathSegment bgp4PathAttrNextHop bgp4PathAttrMultiExitDisc bgp4PathAttrLocalPref\
bgp4PathAttrAtomicAggregate bgp4PathAttrAggregatorAS bgp4PathAttrAggregatorAddr bgp4PathAttrCalcLocalPref\
bgp4PathAttrBest bgp4PathAttrUnknown\n");
	len = strlen(buff);
	while( attrnode ){
		sprintf(buff+len,"%s %s %s %s %s %s %s %s %s %s %s %s %s ?\n",
				attrnode->peer,
				attrnode->addr_prefix_len,
				attrnode->addr_prefix,
				attrnode->origin,
				attrnode->as_path_segment,
				attrnode->nexthop,
				attrnode->multi_exit_disc,
				attrnode->local_pref,
				attrnode->atomic_aggregate,
				attrnode->aggregator_as,
				attrnode->aggregator_addr,
				attrnode->calc_localpref,
				attrnode->best);
		len = strlen(buff);
		attrnode = attrnode->next;
	}
	if( bgp_path_attr_table_last_len != len ){
		bgp_path_attr_table_last_len = len;
		char* newbuff = bgp_path_attr_to_xml(buff);
		module_send_data(newbuff, len, UP_BGP_PATH_TABLE_INFO); 
		free(newbuff);
	}
}
int bgppath_get_and_send(struct bgpPathAttrTable* table)
{
	struct bgpPathAttrTable* attrnode = table->next;
	char buff[10240];
	int len = 0;
	sprintf(buff,"bgp4PathAttrPeer bgp4PathAttrIpAddrPrefixLen bgp4PathAttrIpAddrPrefix bgp4PathAttrOrigin\
bgp4PathAttrASPathSegment bgp4PathAttrNextHop bgp4PathAttrMultiExitDisc bgp4PathAttrLocalPref\
bgp4PathAttrAtomicAggregate bgp4PathAttrAggregatorAS bgp4PathAttrAggregatorAddr bgp4PathAttrCalcLocalPref\
bgp4PathAttrBest bgp4PathAttrUnknown\n");
	len = strlen(buff);
	while( attrnode ){
		sprintf(buff+len,"%s %s %s %s %s %s %s %s %s %s %s %s %s ?\n",
				attrnode->peer,
				attrnode->addr_prefix_len,
				attrnode->addr_prefix,
				attrnode->origin,
				attrnode->as_path_segment,
				attrnode->nexthop,
				attrnode->multi_exit_disc,
				attrnode->local_pref,
				attrnode->atomic_aggregate,
				attrnode->aggregator_as,
				attrnode->aggregator_addr,
				attrnode->calc_localpref,
				attrnode->best);
		len = strlen(buff);
		attrnode = attrnode->next;
	}
	char* newbuff = bgp_path_attr_to_xml(buff);
	module_send_data(newbuff, len, UP_BGP_PATH_TABLE_INFO); 
	free(newbuff);
}
int bgp_path_attr_table_detect(void* args, int type)
{
	if( type == 1)
		get_bgp_path_table(bgppath_diff_and_send);
	if( type == 2)
		get_bgp_path_table(bgppath_get_and_send);
}

int ospf_interface_run_first = 0;
int ospfifinfo_diff_and_send(struct ospfIfTable* table)
{
	int is_changed = 0;
	struct list* iflist = localrouter.iflist;
	if( iflist == NULL ){
		DEBUG(ERROR,"please run update_interface_from_snmp first");
		return -1;
	}
	struct ospfIfTable* ifnode = table->next;
	while(ifnode)
	{
		struct in_addr ipdata;
		inet_pton(AF_INET, ifnode->ipaddress, &ipdata);
        struct list* result = list_new();
		struct rinterface* iface;

        int total = 0;
        get_interface( iflist, Q_IPADDR, &ipdata , &result , &total );
        if ( total >= 1)
        {
            iface = (struct rinterface*)result->head->data;
			int hello_t = atoi(ifnode->hello);
			int dead_t = atoi(ifnode->dead);
			struct ospf_proto_info* p = (struct ospf_proto_info*)(iface->proto_info->info);
			int hello_o = p->hello_interval;
			int dead_o = p->dead_interval;
			if( hello_t != hello_o || dead_o != dead_o  ){
				is_changed = 1;
				p->hello_interval = hello_t;
				p->dead_interval = dead_t;
			}
        }
		else{//find new interface
			
		}
	    list_delete_all_node(result);
	    list_free(result);
		ifnode = ifnode->next;
	}
	if( is_changed ){
		char* buff = ospf_interface_info_to_xml(iflist);
		//send the packet
		int len = strlen(buff);
		module_send_data(buff,len, UP_OSPF_INTERFACE_INFO);
		printf("%s\n",buff);
		free(buff);
		return 0;
	}
	if( ospf_interface_run_first == 0) 
	{
		char* buff = ospf_interface_info_to_xml(iflist);
		int len = strlen(buff);
		module_send_data(buff,len, UP_OSPF_INTERFACE_INFO);
		printf("%s\n", buff);
		free(buff);
		ospf_interface_run_first = 1;
		return 0;
	}
}
int ospfifinfo_get_and_send(struct ospfIfTable* table)
{
	struct list* iflist = localrouter.iflist;
	char* buff = ospf_interface_info_to_xml(iflist);
	//send the packet
	int len = strlen(buff);
	module_send_data(buff,len, UP_OSPF_INTERFACE_INFO);
	free(buff);
}
// OSPF INTERFACE && UPDATE THREAD
int ospf_interface_info_detect(void* args, int type)
{
	if( type == 1)
	get_ospf_if_table(ospfifinfo_diff_and_send);
	if( type == 2)
	get_ospf_if_table(ospfifinfo_get_and_send);
}


int device_info_run_first = 0;
// Hw_STATE effects
int deviceinfo_diff_and_send(struct InterfaceTable* table)
{
	int is_changed = 0;
	struct list* iflist = localrouter.iflist;
	if( iflist == NULL ){
		DEBUG(ERROR,"please run update_interface_from_snmp first");
		return -1;
	}
	
	struct InterfaceTable* ifnode = table->next;
	while(ifnode)
	{
		int i = atoi(ifnode->ifindex);
        struct list* result = list_new();
		struct rinterface* iface;

        int total = 0;
        get_interface( iflist, Q_CTID, &i, &result , &total );
        if ( total >= 1)
        {
            iface = (struct rinterface*)result->head->data;
			int hw_state = check_interface_status( ifnode->ifadminstatus );
			if( hw_state != iface->hw_state ){
				is_changed = 1;
				iface->hw_state = hw_state;
			}
        }
		else{//find new interface
			
		}
	    list_delete_all_node(result);
	    list_free(result);
		ifnode = ifnode->next;
	}
	if( is_changed ){
		char* buff = device_info_to_xml(iflist);
		//send the packet
		int len = strlen(buff);
		module_send_data(buff,len, UP_DEVICE_INFO);
		printf("%s\n",buff);
		free(buff);
	}
	if( device_info_run_first == 0)
	{
		char* buff = device_info_to_xml(iflist);
		int len = strlen(buff);
		module_send_data(buff,len, UP_DEVICE_INFO);
		printf("%s\n", buff);
		free(buff);
		device_info_run_first = 1;
	}
}
int deviceinfo_get_and_send(struct InterfaceTable* table)
{
	struct list* iflist = localrouter.iflist;
	if( iflist == NULL ){
		DEBUG(ERROR,"please run update_interface_from_snmp first");
		return -1;
	}
	
		char* buff = device_info_to_xml(iflist);
		//send the packet
		int len = strlen(buff);
		module_send_data(buff,len, UP_DEVICE_INFO);
		printf("%s\n",buff);
		free(buff);
}

// device_info && update_thread
int device_info_detect(void* args, int type)
{
	if( type == 1)
	get_if_table(deviceinfo_diff_and_send);
	if( type == 2)
		get_if_table(deviceinfo_get_and_send);
}

int update_interface_from_snmp(char *routerip)
{
    struct list *iflist = localrouter.iflist;
    if( iflist == NULL )
	{
		if_init();
        localrouter.iflist = iflist_init();
	}

    iflist = localrouter.iflist;
	int device_type = localrouter.type;

	char routerid[24];
	if( device_type == OSPF_ROUTER )
	{
	//	getOspfRouterId(routerip,routerid);//get the router id
	//	printf("%s\n",routerid);
		getIfInfos(routerip,&ifList); //get the interface by snmp
		printIftable(&ifList);
	}
	else if ( device_type == BGP_ROUTER )
	{
		getBgpIfInfos(routerip, &ifList);
		char asid[24];
		getBgpLocalAs(routerip,asid);
		localrouter.as_id  = atoi(asid);
	}
	else {}

    if( ifList == NULL )
	{
		DEBUG(INFO, "Empty");
        return -1;
	}

    int total_ifs = 0;

	struct ifTable* node = ifList->next;
	while(node)
	{
		DEBUG(INFO, "New Interface");
        struct rinterface * newif  ;
        struct interface * newdetails  ;

        struct list* result = list_new();
        int total = 0;
        int ctid = atoi(node->ifdex);
        get_interface( iflist, Q_CTID, &ctid, &result , &total );
        if( total == 0 )
        {
            newif = malloc_z(struct rinterface);
            newdetails = if_create();
            newif->details = newdetails;
			interface_add( iflist, newif);
        }
        else 
        {
            newif = (struct rinterface*)(result->head->data);
            newdetails = newif->details;
        }
		list_delete_all_node(result);
		list_free(result);
        //ctl id update
        newif->ctl_id = ctid;
        newdetails->ifindex = ctid;
		DEBUG(INFO, "CTLID: %d", ctid); 

        //if name
        int len = strlen(node->ifDescr);
        strncpy(newdetails->name, node->ifDescr, len+1 );
        strncpy(newif->name , node->ifDescr, len+1);
		DEBUG(INFO, "IFNAME: %s", newif->name); 
        
        //status 
        newdetails->status = check_interface_status(node->ifOperStatus);
        newif->op_state = newdetails->status;
        newif->hw_state = check_interface_status(node->ifAdminStatus);
		DEBUG(INFO, "STATUS: %d", newif->op_state); 

        //bandwidth
        newdetails->bandwidth = atoi(node->ifBandwidth);
		DEBUG(INFO, "BANDWIDTH: %d", newdetails->bandwidth); 

        // connected ip
		int prefixlen  = get_prefix_len(node->netMask);
		struct connected* con_ip = connected_new();
		con_ip->address = (struct prefix*)malloc(sizeof(struct prefix));
		con_ip->address->family = AF_INET;
		con_ip->address->prefixlen = prefixlen;
		inet_pton(AF_INET,node->ip,&con_ip->address->u.prefix4);
		con_ip->ifp = newdetails;
		connected_add(newdetails,con_ip);
		DEBUG(INFO, "IP: %s", node->ip); 

        //protocol infomation
		if ( newif->proto_info == NULL)
			newif->proto_info = malloc_z(struct proto_info);
		struct proto_info* proto = newif->proto_info;
		if ( device_type == BGP_ROUTER )
		{
			proto->proto_type = ROUTER_BGP;
			proto->u.asid = atoi(node->areaId);
			if ( proto->u.asid == localrouter.as_id )
				proto->bgp_type = BGP_IBGP;
			else if ( proto->u.asid != localrouter.as_id )
				proto->bgp_type = BGP_EBGP;
		}
		else if ( device_type == OSPF_ROUTER )
		{
			proto->proto_type = ROUTER_OSPF;
			proto->u.areaid = atoi(node->areaId);
			if( proto->info == NULL)
				proto->info = malloc_z(struct ospf_proto_info);
			((struct ospf_proto_info*)proto->info)->hello_interval = 
				atoi(node->ospfIfHelloInterval);
			((struct ospf_proto_info*)proto->info)->dead_interval = 
				atoi(node->ospfIfDeadInterval);
		}
		else {}
        proto->cost = atoi(node->ospfCost);
	//	DEBUG(INFO, "Area: %d", proto->areaid); 
		DEBUG(INFO, "Cost: %d", proto->cost); 

        // hw addr
        len = strlen(node->physAddress);
        strncpy( newdetails->hw_addr, node->physAddress, len+1 );
		DEBUG(INFO, "HwAdrr: %s", newdetails->hw_addr); 
		
		//neighbor infomation
		if ( newif->neighbor == NULL )
			newif->neighbor = malloc_z(struct neighbor);

		struct in_addr nbrid;
		nbrid.s_addr = 0;
		inet_pton(AF_INET, node->nbrRouteId, &nbrid );
		newif->neighbor->rid = nbrid.s_addr; 
		DEBUG(INFO, "Neighbor ID: %d", newif->neighbor->rid);
		inet_pton(AF_INET, node->ospfNbrIp, &newif->neighbor->router_addr);
		DEBUG(INFO, "Neighbor IP: %s", node->ospfNbrIp);
	    
        newif->type = UNKNOWN;

        node = node->next;
	}
	load_interface_type_from_config();
    localrouter.inited = 1;
}
char* ospf_interface_info_to_xml(struct list* iflist){
	xmlDocPtr doc = create_xml_doc();
	if( iflist == NULL )
		return NULL;
	xmlNodePtr devnode;
	xmlNodePtr childnode;
	ifTable* node = ifList->next;
	
	xmlNodePtr rootnode = create_xml_root_node(doc, "OSPF_INTERFACE");
	struct timeval now;
	gettimeofday(&now, NULL);
	char * time = PRINTTIME(now);
	add_xml_child(rootnode, "timestamp",time); free(time);

	devnode = add_xml_child(rootnode, "device", NULL);
	char* aid[24];
	sprintf(aid,"%d", localrouter.agentid);
	add_xml_child_prop(devnode, "id",aid); 

	char value[50];
	while(node)
	{
		// Query The Latest TimeInterval
        struct list* result = list_new();
		struct rinterface* iface;
		int i = atoi(node->ifdex);
		struct ospf_proto_info* ospf_info = NULL;
        int total = 0;
        get_interface( iflist, Q_CTID, &i, &result , &total );
        if ( total >= 1)
        {
            iface = (struct rinterface*)result->head->data;
			ospf_info = (struct ospf_proto_info*)iface->proto_info->info;
        }
		else return 0;
	    list_delete_all_node(result);
	    list_free(result);
		
		childnode = add_xml_child(devnode, "interface", NULL);
		add_xml_child_prop(childnode, "id", node->ifdex);
		if( localrouter.type == OSPF_ROUTER )
			sprintf(value,"ospf");
		else if( localrouter.type == BGP_ROUTER )
			sprintf(value,"bgp");
		add_xml_child(childnode, "protocol", value);
		add_xml_child(childnode, "area_id", node->areaId);
		sprintf(value, "%d",ospf_info->hello_interval);
		add_xml_child(childnode, "hello_timeval", value);
		memset(value,0,50);
		sprintf(value, "%d",ospf_info->dead_interval);
		add_xml_child(childnode, "dead_timeval", value);
		memset(value,0,50);
		sprintf(value,"%d",iface->type);
		add_xml_child(childnode, "type", value);
		node = node->next;
	}
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

char* device_info_to_xml(struct list* iflist){
	xmlDocPtr doc = create_xml_doc();
	if( iflist == NULL )
		return NULL;
	xmlNodePtr devnode;
	xmlNodePtr childnode;
	
	xmlNodePtr rootnode = create_xml_root_node(doc, "DEVICE_INFO");
	struct timeval now;
	gettimeofday(&now, NULL);
	char * time = PRINTTIME(now);
	add_xml_child(rootnode, "timestamp",time); free(time);

	devnode = add_xml_child(rootnode, "device", NULL);
	char aid[24];
	sprintf(aid,"%d", localrouter.agentid);
	add_xml_child_prop(devnode, "id",aid); 

	ifTable* node = ifList->next;


	char value[50];
	while(node)
	{
		// Query The Latest HW_STATE
        struct list* result = list_new();
		struct rinterface* iface;
		int i = atoi(node->ifdex);

        int total = 0;
        get_interface( iflist, Q_CTID, &i, &result , &total );
        if ( total >= 1)
        {
            iface = (struct rinterface*)result->head->data;
			int hw_state =  iface->hw_state ;
			sprintf(value,"%d",hw_state);
        }
		else return 0;
	    list_delete_all_node(result);
	    list_free(result);
		// Add the Element
		childnode = add_xml_child(devnode, "interface", NULL);
		add_xml_child_prop(childnode, "id", node->ifdex);
		add_xml_child(childnode, "interface_name", node->ifDescr);
		add_xml_child(childnode, "ipv4_address", node->ip);
		add_xml_child(childnode, "ipv4_mask", node->netMask);
		add_xml_child(childnode, "mac_address", node->physAddress);
		add_xml_child(childnode, "if_bandwidth", node->ifBandwidth);
		add_xml_child(childnode, "hw_status", value);
		node = node->next;
	}
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
char* ifrate_info_to_xml(struct list* iflist){
	xmlDocPtr doc = create_xml_doc();
	if( iflist == NULL )
		return NULL;
	xmlNodePtr devnode;
	xmlNodePtr childnode;
	
	xmlNodePtr rootnode = create_xml_root_node(doc, "TRAFFIC_INFO");
	struct timeval now;
	gettimeofday(&now, NULL);
	char * time = PRINTTIME(now);
	add_xml_child(rootnode, "timestamp",time); free(time);

	devnode = add_xml_child(rootnode, "device", NULL);
	char aid[24];
	sprintf(aid,"%d", localrouter.agentid);
	add_xml_child_prop(devnode, "id",aid); 

	ifTable* node = ifList->next;


	char value[50];
	while(node)
	{
        struct list* result = list_new();
		struct rinterface* iface;
		int i = atoi(node->ifdex);

        int total = 0;
        get_interface( iflist, Q_CTID, &i, &result , &total );
        if ( total >= 1)
        {
            iface = (struct rinterface*)result->head->data;
        }
		else return 0;
	    list_delete_all_node(result);
	    list_free(result);
		// Add the Element
		childnode = add_xml_child(devnode, "interface", NULL);
		add_xml_child_prop(childnode, "id", node->ifdex);
		sprintf(value,"%lld",iface->send_rate);
		add_xml_child(childnode, "out_rate", value);
		memset(value, 0x0, 50);
		sprintf(value,"%lld",iface->recv_rate);
		add_xml_child(childnode, "in_rate", value);
		node = node->next;
	}
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

char* bgp_path_attr_to_xml(char* data)
{
	xmlDocPtr doc = create_xml_doc();
	xmlNodePtr devnode;
	xmlNodePtr childnode;
	
	xmlNodePtr rootnode = create_xml_root_node(doc, "SNAPSHOT");
	devnode = add_xml_child(rootnode, "ROUTER", NULL);
	char aid[24];
	sprintf(aid,"%d", localrouter.agentid);
	add_xml_child_prop(devnode, "id",aid); 

	struct timeval now;
	gettimeofday(&now, NULL);
	char * time = PRINTTIME(now);
	add_xml_child(devnode, "timestamp",time); free(time);

	add_xml_child(devnode, "BgpPathAttrTable", data);

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

char *interfaceinfo_to_xml(ifTable *head)
{
	xmlDocPtr doc = create_xml_doc();
	if (head == NULL)
		return NULL;
	xmlNodePtr routernode;
	xmlNodePtr childnode;
	ifTable *node = head->next;

	xmlNodePtr rootnode = create_xml_root_node(doc, "INTERFACE_INFO");
	struct timeval now;
	gettimeofday(&now, NULL);
	char * time = PRINTTIME(now);
	add_xml_child(rootnode, "timestamp",time); free(time);

	routernode = add_xml_child(rootnode, "ROUTER", NULL);
	char rid[24];
	sprintf(rid,"%d", localrouter.routerid);
	add_xml_child_prop(routernode, "id",rid); 

	char value[50];
	while(node)
	{
		childnode = add_xml_child(routernode, "interface", NULL);
		add_xml_child_prop(childnode, "id", node->ifdex);
		add_xml_child(childnode, "interface_name", node->ifDescr);
		add_xml_child(childnode, "interface_ipv4_address", node->ip);
		add_xml_child(childnode, "interface_ipv4_netmask", node->netMask);
		add_xml_child(childnode, "interface_mac", node->physAddress);
		add_xml_child(childnode, "interface_bandwidth", node->ifBandwidth);
		memset(value, 0x00, sizeof(value));
		sprintf(value, "%lld", node->inrate);
		add_xml_child(childnode, "current_send_in_data", value);
		memset(value, 0x00, sizeof(value));
		sprintf(value, "%lld", node->outrate);
		add_xml_child(childnode, "current_send_out_data", value);
		node = node->next;
	}
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

void get_interface_info_from_snmp(char* routerip, char** buff, int* len)
{
//	getIfInfos(routerip,&ifList); //get the interface by snmp
	getIfRate(routerip, ifList, 1);
	*buff = interfaceinfo_to_xml(ifList);
	printf("%s\n",*buff);
	*len = strlen(*buff);
//	printIftable(ifList->next);
}


int bgp_interface_info_to_xml()
{
	xmlDocPtr doc = create_xml_doc();
	xmlNodePtr devnode;
	xmlNodePtr childnode;
	ifTable* node = ifList->next;
	
	xmlNodePtr rootnode = create_xml_root_node(doc, "BGP_INTERFACE");
	struct timeval now;
	gettimeofday(&now, NULL);
	char * time = PRINTTIME(now);
	add_xml_child(rootnode, "timestamp",time); free(time);

	devnode = add_xml_child(rootnode, "device", NULL);
	char* aid[24];
	sprintf(aid,"%d", localrouter.agentid);
	add_xml_child_prop(devnode, "id",aid); 

	char value[50];
	while(node)
	{
		// Query The Latest TimeInterval
        struct list* result = list_new();
		struct rinterface* iface;
		struct proto_info *pinfo;
		int i = atoi(node->ifdex);
        int total = 0;
        get_interface( iflist, Q_CTID, &i, &result , &total );
        if ( total >= 1)
        {
            iface = (struct rinterface*)result->head->data;
			pinfo = iface->proto_info;
        }
		else return 0;
	    list_delete_all_node(result);
	    list_free(result);
		
		childnode = add_xml_child(devnode, "interface", NULL);
		add_xml_child_prop(childnode, "id", node->ifdex);
		add_xml_child(childnode, "protocol", "bgp");
		sprintf(value, "%d", pinfo->bgp_type);
		add_xml_child(childnode, "type", value);
		node = node->next;
	}
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

int send_bgp_interface_info()
{
	char* buff = bgp_interface_info_to_xml();
	int len = strlen(buff);
	module_send_data( buff, len, UP_BGP_INTERFACE_INFO);
}

int bgpifinfo_detect(void* args, int type)
{
	if( type == 1 || type == 2) 
		send_bgp_interface_info();
}
