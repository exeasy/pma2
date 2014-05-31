#include <utils/common.h>
#include <utils/utils.h>
#include <pthread.h>
#include <lib/thread_pool.h>
#include <socket.h>
#include <server.h>
#include <srvconf.h>
#include <control/control.h>
#include <netlink/netlink.h>
#include <netlink/route.h>
#include <router/router.h>
#include <router/interface.h>
#include <router/routertable.h>
#include <snmp/includes/pma_api.h>
#include <snmp/includes/snmpcore.h>
#include <lsd/lsdsrv.h>
#include <logger/logger.h>
#include <snmp/includes/snmpcore.h>
#include "icm.h"


extern struct thread_master * master;
extern struct lsd_router g_lsd_router;

Pool* pool;
pthread_t lsd_thread;
pthread_t rt_thread;
pthread_t spf_thread;
pthread_t ifinfo_thread;
pthread_t devinfo_thread;
pthread_t ospfinfo_thread;
pthread_t bgppath_thread;

struct icm_conf conf;

int icm_init()
{
	xml_init();
    log_type_init();
    log_init();
    module_init(ICM);
    lsd_init();
    lsd_thread = 0;
    set_conf_type(ICM_CONF);
    add_ctl(packet_handler, PMA_LIST, pma_list_handle,1 );
    add_ctl(packet_handler, ACK,      ack_handle, 1);
    add_ctl(packet_handler, ICM_CONF, conf_handle, 1);
	pool = malloc_z(Pool);
	init_pthread_pool(pool);
	start_pthread_pool(pool);
}

    int
icm_start()
{
    module_start();
}
extern itemopt ospfrouterid;
extern itemopt bgpidentifier;


int delay_time(int n)//ms
{
	struct timeval delay;
	delay.tv_sec = n/1000;
	delay.tv_usec = n%1000 * 1000; // n ms
	select(0, NULL, NULL, NULL, &delay);
}

void bgp_path_attr_table_detect_daemon()
{
	while(1){
		bgp_path_attr_table_detect_thread();
		delay_time(1000);
	}
}

void interface_detect_daemon()
{
	while(1)
	{
		device_info_update_thread();
		delay_time(1000);
	}
}
void ospf_interface_detect_daemon()
{
	while(1)
	{
		ospf_interface_info_update_thread();
		delay_time(1000);
	}
}

void route_table_thread(void* args)
{
	if ( args == NULL ) return NULL;
	struct route_table* rtable  = (struct route_table*)args;
	route_table_get(rtable);
	int ret = watch_route_change(rtable->routes);
	if ( ret )
		send_route_table(rtable);
	refresh_route_status(rtable->routes);
}

int route_table_daemon()
{
	struct route_table* rtable = 
		malloc_z(struct route_table);
	if ( conf.outside == 1 )//outside pma only support get the routetable by snmp
	{
		rtable->acquire_type = SNMP_ROUTE;
		inet_ntop(AF_INET, &conf.router_ip,rtable->routerip,24); 
	}
	else // only if the inside pma ,we can use netlink, :)
	{
		rtable->acquire_type = NETLINK_ROUTE;
	}
	rtable->routes = malloc_z(struct route);
	while(1)
	{
		route_table_thread(rtable);
	//	add_task_to_pool(pool, route_table_thread, (void*)rtable);
		delay_time(1000);
	}
}

void ifinfo_thread_function(void *args)
{
	if( args == NULL) return NULL;
	char* routerip = (char*)args;
	char* buff; int len;
	get_interface_info_from_snmp(routerip, &buff, &len);
	module_send_data(buff, len, UP_TRAFFICE_INFO); 
	free(buff);
}
int ifinfo_daemon()
{
	char *routerip = (char*)malloc(24);	
	inet_ntop(AF_INET, &conf.router_ip,routerip,24); 
	while(1)
	{
		ifrate_info_detect_daemon();
//		ifinfo_thread_function(routerip);
//		add_task_to_pool(pool, ifinfo_thread_function, (void*)routerip);
		printf("Put the ifinfo task into pool\n");
		delay_time(2000);
	}
}

void spf_count_thread(void *args)
{
	if ( args == NULL ) return NULL;
	int* arg = (int*)args;
	int *azs = (int*)arg[0];
	int count = arg[1];
	int *spfcount =(int*)arg[2];
	char routerip[24];
	inet_ntop(AF_INET, &conf.router_ip, routerip, 24);
	int i  = 0;
	for ( ;i < count ; i++)
	{
		char areaid[24];
		inet_ntop(AF_INET, &azs[i], areaid, 24);
		int spfc = getRouterSpfCount(routerip, areaid);	
		if ( spfc != spfcount[i] )
		{
			spfcount[i] = spfc;
			char *buff = (char*)malloc(sizeof(int));
			memcpy(buff, &azs[i], sizeof(int));
			module_send_data(buff, sizeof(int), OSPF_SPF);
			free(buff);
		}
	}
}

int spf_count_daemon()
{
	int count = 0;
	int* azs = get_all_areas(&count);
	int args[2];
	int spfcount[count];
	args[0] = (int)azs;
	args[1] = count;
	args[2] = (int)spfcount;
	while(1)
	{
		spf_count_thread(args);
//		add_task_to_pool(pool, spf_count_thread, (void*)args);
		delay_time(1000);
	}
}

int send_route_table(struct route_table * rt)
{
	struct route* route = rt->routes;
	printf("Route Table Len: %d\n",rt->route_table_len);
	char *msg = (char*)malloc(rt->route_table_len* 80);
	memset(msg,0x00, rt->route_table_len*80);
	char * buff = msg;
	int len;
	sprintf(msg, "Destination Mask Gateway Nexthop Type Metric\n");
	len = strlen(msg);
	while(route)
	{
		char * c= get_single_route(route);
		sprintf(buff+len, "%s\n",c);
		len = strlen(msg);
		route = route->next;
	}
	printf("%s\n",msg);
	module_send_data(msg, len, UP_ROUTE_INFO);
	free(msg);
}


    int
pma_list_handle( struct Packet_header *pkt )
{
    char *data = pkt->pkt;
    int length = pkt->pkt_len;
	int i;
	int count = length/sizeof(struct pma_list);
	struct pma_list* pmalist = (struct pma_list*)data;
    for( i = 0; i< count ; i++)
    {
        int devid = pmalist[i].device_id;
		int rid = pmalist[i].router_id;
		struct in_addr ip = pmalist[i].ip;
        set_neighbor_agent_address(devid, rid, ip);
    }
    lsd_iflist_init();
	// lsd 采用了zebra 伪线程，这里没有使用timer定时器，而是直接新建了线程，在线程内部无限循环
    if( lsd_thread == 0 )
	    pthread_create(&lsd_thread ,NULL,  lsd_start ,  NULL);
}
    int
conf_handle( struct Packet_header *pkt )
{
    if ( pkt->pkt_len == 0) return 0;
	memcpy(&conf,pkt->pkt,pkt->pkt_len);
	int self_id  = conf.pma_id;
	//router id 
	char routerid[24];
	//local ip
    char local_ip[24];
	//local address mask
    char local_mask[24];
	//router ip
	static char routerip[24];

	inet_ntop(AF_INET,&conf.local_ip,local_ip,24);
	inet_ntop(AF_INET,&conf.netmask,local_mask,24);
	inet_ntop(AF_INET,&conf.router_ip,routerip,24);	
	DEBUG(INFO,"Receive Config:\nRouterID[%d]\nRouterIP:[%s]\nPMAIP[%s]\nPMANETMASK[%s]\n",\
		self_id,routerip,local_ip,local_mask);
	struct in_addr pma_addr;
	inet_pton(AF_INET,local_ip,&pma_addr);//local_ip come from the config recv by the basemodule.

	snmp_init(routerip);

	//device type ( 1: ospf 2: bgp 3:both)
	int type = conf.device_type;


	if( type == OSPF_ROUTER )
	{
		get_ospf_routerid(routerid);
	}
//	getOspfRouterId(routerip,routerid);//get the router id
	else if ( type == BGP_ROUTER )
	{
		get_bgp_routerid(routerid);
	}
//	getBgpRouterId(routerip, routerid);
	else {}

	int rid = 0 ;
	inet_pton(AF_INET, routerid, &rid);


    init_local_router(rid , self_id, conf.router_ip, type);
	
    update_interface_from_snmp(routerip);
	if ( rt_thread == 0 )
		pthread_create(&rt_thread, NULL, route_table_daemon , NULL);
	if ( ifinfo_thread == 0 )
		pthread_create(&ifinfo_thread, NULL, ifinfo_daemon, NULL);
	if ( devinfo_thread == 0 )
		pthread_create(&devinfo_thread, NULL, interface_detect_daemon, NULL);

	if ( type == OSPF_ROUTER )
	{
	if ( spf_thread == 0 )
		pthread_create(&spf_thread, NULL, spf_count_daemon, NULL);
	if ( ospfinfo_thread == 0 )
		pthread_create(&ospfinfo_thread, NULL, ospf_interface_detect_daemon, NULL);
	}

	if ( type == BGP_ROUTER )
	{
	if ( bgppath_thread == 0 )
		pthread_create(&bgppath_thread, NULL, bgp_path_attr_table_detect_daemon, NULL);
	send_bgp_interface_info();
	}
}
