#include <utils/utils.h>
#include <utils/common.h>
#include <netlink/netlink.h>
#include <netlink/route.h>
#include <snmp/includes/pma_api.h>
#include <routertable.h>

//netlink route table
extern struct route* route_list;

int route_table_get(struct route_table * rt)
{
	if ( rt == NULL ) return -1;
	int type  = rt->acquire_type;
	switch (type)
	{
		case NETLINK_ROUTE:
			get_route_table_by_netlink(rt);break;
		case SNMP_ROUTE:
			get_route_table_by_snmp(rt);break;
		default:break;
	}
}



int get_route_table_by_netlink(struct route_table *rt)
{
	struct nlsock nl;
	int ret = netlink_socket(&nl, 0 );
	get_route_table(nl);
	int i = 0;
	struct route* r = route_list;
	while(r)
	{
		i++;
		r = r->next;
	}
	rt->route_table_len = i;
	rt->routes = route_list;
    close(nl.sock);
}

int get_prefix_length(char* mask){
	if(*mask=='\0')return 0;
	int host_order = 0;
	if(mask[0] == '0' ) // if the format is 0.255.255.255, then convert to the host order byte
	{
		host_order = 1;
	}
	struct in_addr n_mask;
	inet_aton(mask,&n_mask);
	u32 b_mask = (u32)n_mask.s_addr;
	if( host_order )
		b_mask = ntohl(b_mask);
	b_mask = ~b_mask;
	int count = 0;
	while(b_mask){
		b_mask = b_mask<<1;
		count++;
	}
	return 32-count;
}

int  get_route_table_by_snmp(struct route_table *rt)
{
	ipRouteTable *rtable = NULL ;
//	getIpRouteTable(rt->routerip, &rtable); 
	getIpForwardTable(rt->routerip, &rtable); 
	if(rtable == NULL)
	{
		printf("Get routing table failed\n");
		return NULL;
	}
	ipRouteTable* r = rtable->next;
	int total = 0;
	while(r!=NULL){
		struct route* newroute = malloc_z(struct route); 

		inet_pton(AF_INET, r->ipRouteDest ,&newroute->prefix.prefix);
		newroute->prefix.family = AF_INET;
		newroute->prefix.prefixlen = get_prefix_length(r->ipRouteMask);

		inet_pton(AF_INET, r->ipRouteNextHop, &newroute->gateway);

		newroute->interface_id = atoi(r->ipRouteIfIndex);

		newroute->metric = atoi(r->ipRouteMetric);

		if ( strcmp(r->ipRouteType,"local") == 0 ) 
		newroute->type = ROUTE_TYPE_LOCAL;
		else 
			newroute->type = ROUTE_TYPE_REMOTE;
		
		newroute->dirty = FRESH_STATUS;
		
		struct route** rr = find_before_route(rt->routes, *newroute);
		if( rr== NULL)
			add_route(rt->routes, newroute);
		else
			update_route(rr, newroute);

		total++;
		r = r->next;
	}
	freeIpRouteTable(rtable);
	rt->route_table_len = total;
}
