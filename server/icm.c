#include <utils/common.h>
#include <utils/utils.h>
#include <utils/thread_daemon.h>
#include <utils/ini.h>
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
PDaemon routetable_daemon; //route table detection 
PDaemon ifinfo_daemon;		//interface info detection
PDaemon devinfo_daemon;		//device_info detection
PDaemon spf_daemon;			//spf signal detection
PDaemon ospfifinfo_daemon; //ospf ifinfo detection
PDaemon bgpifinfo_daemon; //bgp ifinfo detection
PDaemon bgppath_daemon;		//bgp path table detection

pthread_t lsd_thread; //link detection thread

int daemon_started = 0;


struct icm_conf conf;

int icm_init()
{
	parse_ini("icm.ini", ini_parser, NULL);

	xml_init();
    log_type_init();
    log_init();
    module_init(ICM);
    lsd_init();
    //lsd_thread = 0;
    set_conf_type(ICM_CONF);
    add_ctl(packet_handler, PMA_LIST, pma_list_handle,1 );
    add_ctl(packet_handler, ACK,      ack_handle, 1);
    add_ctl(packet_handler, ICM_CONF, conf_handle, 1);
//	pool = malloc_z(Pool);
//	init_pthread_pool(pool);
//	start_pthread_pool(pool);
	detect_daemon_init();
}

    int
icm_start()
{
    module_start();
}
extern itemopt ospfrouterid;
extern itemopt bgpidentifier;


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
	//hello ip
	char hello_ip[24];
	//hello address mask
    char hello_mask[24];
	//router ip
	static char routerip[24];

	inet_ntop(AF_INET,&conf.local_ip,local_ip,24);
	inet_ntop(AF_INET,&conf.hello_ip,hello_ip,24);
	inet_ntop(AF_INET,&conf.netmask,hello_mask,24);
	inet_ntop(AF_INET,&conf.router_ip,routerip,24);	
	DEBUG(INFO,"Receive Config:\nRouterID[%d]\nRouterIP:[%s]\nHelloIP[%s]\nHelloMASK[%s]\n",\
		self_id,routerip,hello_ip,hello_mask);

	snmp_init(routerip);

	//protocl type ( 1: ospf 2: bgp 3:both)
	int type = conf.device_type%10;


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

	if( daemon_started == 1) 
		return 0;

	if (daemon_started == 0 )
		daemon_started = 1;

	//route table detection
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
	routetable_daemon.args = (void*)rtable;
	create_daemon(&routetable_daemon);
	//[end]route table detection 


	// if info detect
	create_daemon(&ifinfo_daemon);

	// device info detect
	create_daemon(&devinfo_daemon);

	if ( type == OSPF_ROUTER )
	{
		//ospf spf signal detect
		int count = 0;
		int* azs = get_all_areas(&count);
		int* args = (int*)malloc(2*sizeof(int));
		int spfcount = (int*)malloc(count*sizeof(int));
		args[0] = (int)azs;
		args[1] = count;
		args[2] = (int)spfcount;
		spf_daemon.args = (void*)args;
		create_daemon(&spf_daemon);

		//ospfifinfo detect
		create_daemon(&ospfifinfo_daemon);
	}

	if ( type == BGP_ROUTER )
	{
		//bgpifinfo detect
		create_daemon(&bgpifinfo_daemon);
		
		//bgppath_thread detect
		create_daemon(&bgppath_daemon);
	}
}



#define IS_VAL(s,k) ({ int ret= 0; ret = (strcmp(section, s) == 0 && strcmp( key, k ) == 0) ; ret; })


int ini_parser(const char* file, int line, const char* section, char *key, char* value, void* data)
{
	if ( section != NULL && key != NULL && value != NULL )
		printf( "Section %s -> key %s:value %s\n", section, key, value);
	else return 0;
	if(IS_VAL("routetable","min_interval")){
		routetable_daemon.min_interval = adv_atoi(value,10);
		return 0;
	}
	if(IS_VAL("routetable","max_interval")) {
		routetable_daemon.max_interval = adv_atoi(value,10);
		return 0;
	}

	if(IS_VAL("ifinfo","min_interval")){
		ifinfo_daemon.min_interval = adv_atoi(value,10);
		return 0;
	}
	if(IS_VAL("ifinfo","max_interval")) {
		ifinfo_daemon.max_interval = adv_atoi(value,10);
		return 0;
	}

	if(IS_VAL("devinfo","min_interval")){
		devinfo_daemon.min_interval = adv_atoi(value,10);
		return 0;
	}
	if(IS_VAL("devinfo","max_interval")) {
		devinfo_daemon.max_interval = adv_atoi(value,10);
		return 0;
	}

	if(IS_VAL("spf","min_interval")){
		spf_daemon.min_interval = adv_atoi(value,10);
		return 0;
	}
	if(IS_VAL("spf","max_interval")) {
		spf_daemon.max_interval = adv_atoi(value,10);
		return 0;
	}

	if(IS_VAL("ospf_ifinfo","min_interval")){
		ospfifinfo_daemon.min_interval = adv_atoi(value,10);
		return 0;
	}
	if(IS_VAL("ospf_ifinfo","max_interval")) {
		ospfifinfo_daemon.max_interval = adv_atoi(value,10);
		return 0;
	}

	if(IS_VAL("bgp_ifinfo","min_interval")){
		bgpifinfo_daemon.min_interval = adv_atoi(value,10);
		return 0;
	}
	if(IS_VAL("bgp_ifinfo","max_interval")) {
		bgpifinfo_daemon.max_interval = adv_atoi(value,10);
		return 0;
	}

	if(IS_VAL("bgp_pathinfo","min_interval")){
		bgppath_daemon.min_interval = adv_atoi(value,10);
		return 0;
	}
	if(IS_VAL("bgp_pathinfo","max_interval")) {
		bgppath_daemon.max_interval = adv_atoi(value,10);
		return 0;
	}
	return 0;
}

int route_table_detect(void* args, int type)
{
	if ( args == NULL ) return NULL;
	struct route_table* rtable  = (struct route_table*)args;
	route_table_get(rtable);
	int ret = watch_route_change(rtable->routes);
	if ( ret || type == 2)
		send_route_table(rtable);
	refresh_route_status(rtable->routes);
}



int spf_detect(void *args, int type)
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
		if ( spfc != spfcount[i] || type == 2)
		{
			spfcount[i] = spfc;
			char *buff = (char*)malloc(sizeof(int));
			memcpy(buff, &azs[i], sizeof(int));
			module_send_data(buff, sizeof(int), OSPF_SPF);
			free(buff);
		}
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

int detect_daemon_init()
{
	routetable_daemon.daemon  =  route_table_detect;
	ifinfo_daemon.daemon =  ifrate_detect;
	devinfo_daemon.daemon = device_info_detect; 
	spf_daemon.daemon = spf_detect;
	ospfifinfo_daemon.daemon = ospf_interface_info_detect;
	bgpifinfo_daemon.daemon = bgpifinfo_detect;
	bgppath_daemon.daemon = bgp_path_attr_table_detect;
}
