#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "includes/snmpcore.h"
#include "includes/oids.h"
#include "includes/typemapper.h"
typedef unsigned int u32;
tableopt iftable = { 
	.column = 10, 
	.mapper = iftable_mapper, 
	.oids = IFTABLE, 
	.tablehead = NULL, 
	.type_size = sizeof(struct InterfaceTable) ,
	.lock = PTHREAD_MUTEX_INITIALIZER
};

tableopt ipaddrtable = { 
	.column = 4, 
	.mapper = ipaddr_mapper, 
	.oids = IPADDRTABLE, 
	.tablehead = NULL, 
	.type_size = sizeof(struct IpAddrTable), 
	.lock = PTHREAD_MUTEX_INITIALIZER
};

tableopt iproutetable = { 
	.column = 10, 
	.mapper = iproute_mapper, 
	.oids = IPFORWARDTABLE, 
	.tablehead = NULL, 
	.type_size = sizeof(struct IpRouteTable),
	.lock = PTHREAD_MUTEX_INITIALIZER
};
tableopt ospfareatable = { 
	.column = 4, 
	.mapper = ospfarea_mapper, 
	.oids = OSPFAREATABLE, 
	.tablehead = NULL, 
	.type_size = sizeof(struct ospfAreaTable), 
	.lock = PTHREAD_MUTEX_INITIALIZER
};
tableopt ospfiftable = { 
	.column = 14, 
	.mapper = ospfif_mapper, 
	.oids = OSPFIFTABLE, 
	.tablehead = NULL, 
	.type_size = sizeof(struct ospfIfTable) ,
	.lock = PTHREAD_MUTEX_INITIALIZER
};
tableopt ospfifmetrictable = {
	.column = 3,
	.mapper = ospfifmetric_mapper,
	.oids = OSPFIFMETRICTABLE,
	.tablehead = NULL,
	.type_size = sizeof(struct ospfIfMetricTable),
	.lock = PTHREAD_MUTEX_INITIALIZER
};
tableopt ospfneighbortable = {
	.column = 5,
	.mapper = ospfneighbor_mapper,
	.oids = OSPFNBRTABLE,
	.tablehead = NULL,
	.type_size = sizeof(struct ospfNeighborTable),
	.lock = PTHREAD_MUTEX_INITIALIZER
};
tableopt bgppeertable = {
	.column = 21,
	.mapper = bgppeer_mapper,
	.oids = BGPPEERTABLE,
	.tablehead = NULL,
	.type_size = sizeof(struct bgpPeerTable),
	.lock = PTHREAD_MUTEX_INITIALIZER
};
tableopt bgprecvpathtable = {
	.column = 6,
	.mapper = bgprecvpath_mapper,
	.oids = BGPRCVDPATHATTRTABLE,
	.tablehead = NULL,
	.type_size = sizeof(struct bgpRecvPathAttrTable),
	.lock = PTHREAD_MUTEX_INITIALIZER
};
tableopt bgppathtable = {
	.column = 13,
	.mapper = bgppath_mapper,
	.oids = BGP4PATHATTRTABLE,
	.tablehead = NULL,
	.type_size = sizeof(struct bgpPathAttrTable),
	.lock = PTHREAD_MUTEX_INITIALIZER
};
itemopt ospfrouterid = {
	.oids = OSPFROUTERID,
	.itemhead = NULL,
	.lock = PTHREAD_MUTEX_INITIALIZER
};
itemopt ospfversion = {
	.oids = OSPFVERSION,
	.itemhead = NULL,
	.lock = PTHREAD_MUTEX_INITIALIZER
};
itemopt bgpversion = {
	.oids = BGPVERSION,
	.itemhead = NULL,
	.lock = PTHREAD_MUTEX_INITIALIZER
};
itemopt bgplocalas = {
	.oids = BGPLOCALAS,
	.itemhead = NULL,
	.lock = PTHREAD_MUTEX_INITIALIZER
};
itemopt bgpidentifier = {
	.oids = BGPIDENTIFIER,
	.itemhead = NULL,
	.lock = PTHREAD_MUTEX_INITIALIZER
};

session_id_t global_snmp_session;
extern session_id_t create_session(char* ip, char* community);

int snmp_init(const char* routerip)
{
	global_snmp_session  = create_session(routerip, "public");
	return 0;
}

int is_inited(){
	return global_snmp_session;
}

int get_ospf_routerid(char* routerid)
{
	if( routerid == NULL && is_inited() == 0) return -1;
	pthread_mutex_lock(&ospfrouterid.lock);
	int ret = update_item(global_snmp_session, &ospfrouterid);
	if ( 0 == ret ){//no error
		strcpy(routerid, ospfrouterid.itemhead);
		clear_item(&ospfrouterid);
		pthread_mutex_unlock(&ospfrouterid.lock);
		return 0;
	}
	else if ( -1 == ret ) {//data format error
		clear_item(&ospfrouterid);
		pthread_mutex_unlock(&ospfrouterid.lock);
		return -1;
	}
	else {//no data acquired
		pthread_mutex_unlock(&ospfrouterid.lock);
		return -1;
	}
}
int get_bgp_routerid(char* routerid)
{
	if( routerid == NULL && is_inited() == 0) return -1;
	pthread_mutex_lock(&bgpidentifier.lock);
	int ret = update_item(global_snmp_session, &bgpidentifier);
	if ( 0 == ret ) {
		strcpy(routerid, bgpidentifier.itemhead);
		clear_item(&bgpidentifier);
		pthread_mutex_unlock(&bgpidentifier.lock);
		return 0;
	}
	else if ( -1 == ret ) {
		clear_item(&bgpidentifier);
		pthread_mutex_unlock(&bgpidentifier.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&bgpidentifier.lock);
		return -1;
	}
}
int get_bgp_asid(char* asid)
{
	if( asid == NULL && is_inited() == 0) return -1;
	pthread_mutex_lock(&bgplocalas.lock);
	int ret = update_item(global_snmp_session, &bgplocalas);
	if ( 0 == ret ){
		strcpy(asid, bgplocalas.itemhead);
		clear_item(&bgplocalas);
		pthread_mutex_unlock(&bgplocalas.lock);
		return 0;
	}
	else if ( -1 == ret ) {
		clear_item(&bgplocalas);
		pthread_mutex_unlock(&bgplocalas.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&bgplocalas.lock);
		return -1;
	}
}

int get_bgp_peer_table(callbackptr callback)
{
	if( is_inited() == 0) return -1;
	pthread_mutex_lock(&bgppeertable.lock);
	int ret = update_table(global_snmp_session, &bgppeertable);
	if ( 0 == ret ){
		ret =callback((void*)bgppeertable.tablehead);
		clear_table(&bgppeertable);
		pthread_mutex_unlock(&bgppeertable.lock);
		return ret;
	}
	else if ( -1 == ret ) {
		clear_table(&bgppeertable);
		pthread_mutex_unlock(&bgppeertable.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&bgppeertable.lock);
		return -1;
	}
}
int get_bgp_path_table(callbackptr callback)
{
	if( is_inited() == 0) return -1;
	pthread_mutex_lock(&bgppathtable.lock);
	int ret = update_table(global_snmp_session, &bgppathtable);
	if ( 0 == ret ){
		ret =callback((void*)bgppathtable.tablehead);
		clear_table(&bgppathtable);
		pthread_mutex_unlock(&bgppathtable.lock);
		return 0;
	}
	else if ( -1 == ret ) {
		clear_table(&bgppathtable);
		pthread_mutex_unlock(&bgppathtable.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&bgppathtable.lock);
		return -1;
	}
}
int get_if_table(callbackptr callback)
{
	if( is_inited() == 0) return -1;
	pthread_mutex_lock(&iftable.lock);
	int ret = update_table(global_snmp_session, &iftable);
	if ( 0 == ret ){
		ret =callback((void*)iftable.tablehead);
		clear_table(&iftable);
		pthread_mutex_unlock(&iftable.lock);
		return 0;
	}
	else if ( -1 == ret ) {
		clear_table(&iftable);
		pthread_mutex_unlock(&iftable.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&iftable.lock);
		return -1;
	}
}
int get_ipaddr_table(callbackptr callback)
{
	if( is_inited() == 0) return -1;
	pthread_mutex_lock(&ipaddrtable.lock);
	int ret = update_table(global_snmp_session, &ipaddrtable);
	if ( 0 == ret ){
		ret =callback((void*)ipaddrtable.tablehead);
		clear_table(&ipaddrtable);
		pthread_mutex_unlock(&ipaddrtable.lock);
		return 0;
	}
	else if ( -1 == ret ) {
		clear_table(&ipaddrtable);
		pthread_mutex_unlock(&ipaddrtable.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&ipaddrtable.lock);
		return -1;
	}
}
int get_iproute_table(callbackptr callback)
{
	if( is_inited() == 0) return -1;
	pthread_mutex_lock(&iproutetable.lock);
	int ret = update_table(global_snmp_session, &iproutetable);
	if ( 0 == ret ){
		ret =callback((void*)iproutetable.tablehead);
		clear_table(&iproutetable);
		pthread_mutex_unlock(&iproutetable.lock);
		return ret;
	}
	else if ( -1 == ret ) {
		clear_table(&iproutetable);
		pthread_mutex_unlock(&iproutetable.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&iproutetable.lock);
		return -1;
	}
}
int get_ospf_area_table(callbackptr callback)
{
	if( is_inited() == 0) return -1;
	pthread_mutex_lock(&ospfareatable.lock);
	int ret = update_table(global_snmp_session, &ospfareatable);
	if ( 0 == ret ){
		ret =callback((void*)ospfareatable.tablehead);
		clear_table(&ospfareatable);
		pthread_mutex_unlock(&ospfareatable.lock);
		return ret;
	}
	else if ( -1 == ret ) {
		clear_table(&ospfareatable);
		pthread_mutex_unlock(&ospfareatable.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&ospfareatable.lock);
		return -1;
	}
}
int get_ospf_ifmetric_table(callbackptr callback)
{
	if( is_inited() == 0) return -1;
	pthread_mutex_lock(&ospfifmetrictable.lock);
	int ret = update_table(global_snmp_session, &ospfifmetrictable);
	if ( 0 == ret ){
		ret =callback((void*)ospfifmetrictable.tablehead);
		clear_table(&ospfifmetrictable);
		pthread_mutex_unlock(&ospfifmetrictable.lock);
		return ret;
	}
	else if ( -1 == ret ) {
		clear_table(&ospfifmetrictable);
		pthread_mutex_unlock(&ospfifmetrictable.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&ospfifmetrictable.lock);
		return -1;
	}
}
int get_ospf_if_table(callbackptr callback)
{
	if( is_inited() == 0) return -1;
	pthread_mutex_lock(&ospfiftable.lock);
	int ret = update_table(global_snmp_session, &ospfiftable);
	if ( 0 == ret ){
		ret =callback((void*)ospfiftable.tablehead);
		clear_table(&ospfiftable);
		pthread_mutex_unlock(&ospfiftable.lock);
		return ret;
	}
	else if ( -1 == ret ) {
		clear_table(&ospfiftable);
		pthread_mutex_unlock(&ospfiftable.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&ospfiftable.lock);
		return -1;
	}
}
int get_ospf_neighbor_table(callbackptr callback)
{
	if( is_inited() == 0) return -1;
	pthread_mutex_lock(&ospfneighbortable.lock);
	int ret = update_table(global_snmp_session, &ospfneighbortable);
	if ( 0 == ret ){
		ret =callback((void*)ospfneighbortable.tablehead);
		clear_table(&ospfneighbortable);
		pthread_mutex_unlock(&ospfneighbortable.lock);
		return ret;
	}
	else if ( -1 == ret ) {
		clear_table(&ospfneighbortable);
		pthread_mutex_unlock(&ospfneighbortable.lock);
		return -1;
	}
	else {
		pthread_mutex_unlock(&ospfneighbortable.lock);
		return -1;
	}
}

session_id_t create_session(char* ip, char* community){
	if( ip == NULL || community == NULL ) return (session_id_t)(-1);
	snmpsession* s = (snmpsession*)malloc(sizeof(snmpsession));
	strcpy(s->ip,ip);
	strcpy(s->community,community);
	return (session_id_t)s;
}

int show_interface_table(struct InterfaceTable* tablehead) {
	struct InterfaceTable* handle = tablehead->next;
	while(handle){
		printf("%s\n",handle->ifindex);
		printf("%s\n",handle->ifdescr);
		printf("%s\n",handle->iftype);
		printf("%s\n",handle->ifmtu);
		printf("%s\n",handle->ifspeed);
		printf("%s\n",handle->ifphysaddress);
		printf("%s\n",handle->ifadminstatus);
		printf("%s\n",handle->ifoperstatus);
		printf("%s\n",handle->ifinoctets);
		printf("%s\n",handle->ifoutoctets);
		handle = handle->next;
	}
}
int show_ipaddress_table(struct IpAddrTable* tablehead) {
	struct IpAddrTable* handle = tablehead->next;
	while(handle){
		printf("%s\n",handle->ifindex);
		printf("%s\n",handle->ipaddr);
		printf("%s\n",handle->ipmask);
		printf("%s\n",handle->bcastaddr);
		handle = handle->next;
	}
}
int show_iproute_table(struct IpRouteTable* tablehead) {
	struct IpRouteTable* handle = tablehead->next;
	while(handle){
		printf("%s ",handle->dest);
		printf("%s ",handle->mask);
		printf("%s ",handle->tos);
		printf("%s ",handle->nexthop);
		printf("%s ",handle->ifindex);
		printf("%s ",handle->type);
		printf("%s ",handle->proto);
		printf("%s ",handle->nextas);
		printf("%s ",handle->metric);
		printf("%s\n",handle->status);
		handle = handle->next;
	}
}
int show_ospfarea_table(struct ospfAreaTable* tablehead) {
	struct ospfAreaTable* handle = tablehead->next;
	while(handle){
		printf("%s\n",handle->areaid);
		printf("%s\n",handle->spfruns);
		printf("%s\n",handle->lsacount);
		printf("%s\n",handle->status);
		handle = handle->next;
	}
}
int show_ospfif_table(struct ospfIfTable* tablehead) {
	struct ospfIfTable* handle = tablehead->next;
	while(handle){
		printf("%s\n",handle->ipaddress);
		printf("%s\n",handle->areaid);
		printf("%s\n",handle->type);
		printf("%s\n",handle->adminstatus);
		printf("%s\n",handle->transit);
		printf("%s\n",handle->retransit);
		printf("%s\n",handle->hello);
		printf("%s\n",handle->dead);
		printf("%s\n",handle->pool);
		printf("%s\n",handle->state);//dr or bdr
		printf("%s\n",handle->drouter);
		printf("%s\n",handle->bdrouter);
		printf("%s\n",handle->ifevent);
		printf("%s\n",handle->status);
		handle = handle->next;
	}
}
int show_ospfifmetric_table(struct ospfIfMetricTable* tablehead) {
	struct ospfIfMetricTable* handle = tablehead->next;
	while(handle){
		printf("%s\n",handle->ipaddress);
		printf("%s\n",handle->metric);
		printf("%s\n",handle->status);
		handle = handle->next;
	}
}
int show_ospfneighbor_table(struct ospfNeighborTable* tablehead) {
	struct ospfNeighborTable* handle = tablehead->next;
	while(handle){
		printf("%s\n",handle->nbrip);
		printf("%s\n",handle->nbrid);
		printf("%s\n",handle->state);
		printf("%s\n",handle->events);
		printf("%s\n",handle->status);
		handle = handle->next;
	}
}
int show_bgppeer_table(struct bgpPeerTable* tablehead) {
	struct bgpPeerTable* handle = tablehead->next;
	while(handle){
		printf("%s\n",handle->identifier);
		printf("%s\n",handle->peerstate);
		printf("%s\n",handle->adminstatus);
		printf("%s\n",handle->localaddr);
		printf("%s\n",handle->localport);
		printf("%s\n",handle->peeraddr);
		printf("%s\n",handle->peerport);
		printf("%s\n",handle->remoteas);
		printf("%s\n",handle->inupdates);
		printf("%s\n",handle->outupdates);
		printf("%s\n",handle->inmessages);
		printf("%s\n",handle->outmessages);
		printf("%s\n",handle->estabtime);
		printf("%s\n",handle->conretrytime);
		printf("%s\n",handle->holdtime);
		printf("%s\n",handle->keepalive);
		printf("%s\n",handle->holdtime_conf);
		printf("%s\n",handle->keepalive_conf);
		printf("%s\n",handle->minasorigination);
		printf("%s\n",handle->minrouteadvertise);
		printf("%s\n",handle->updateelapsed);
		handle = handle->next;
	}
}
int show_bgprecvpath_table(struct bgpRecvPathAttrTable* tablehead) {
	struct bgpRecvPathAttrTable* handle = tablehead->next;
	while(handle){
		printf("%s\n",handle->peer);
		printf("%s\n",handle->destnetwork);
		printf("%s\n",handle->origin);
		printf("%s\n",handle->aspath);
		printf("%s\n",handle->nexthop);
		printf("%s\n",handle->inter_as_metric);
		handle = handle->next;
	}
}
int show_bgppath_table(struct bgpPathAttrTable* tablehead) {
	struct bgpPathAttrTable* handle = tablehead->next;
	while(handle){
		printf("%s\n",handle->peer);
		printf("%s\n",handle->addr_prefix_len);
		printf("%s\n",handle->addr_prefix);
		printf("%s\n",handle->origin);
		printf("%s\n",handle->as_path_segment);
		printf("%s\n",handle->nexthop);
		printf("%s\n",handle->multi_exit_disc);
		printf("%s\n",handle->local_pref);
		printf("%s\n",handle->atomic_aggregate);
		printf("%s\n",handle->aggregator_as);
		printf("%s\n",handle->aggregator_addr);
		printf("%s\n",handle->calc_localpref);
		printf("%s\n",handle->best);
		handle = handle->next;
	}
}
int clear_item(itemopt* item)
{
	if( item->itemhead != NULL)
	{
		free(item->itemhead);
		item->itemhead = NULL;
		return 0;
	}
	return 0;
}
int update_item(session_id_t s, itemopt* item)
{
	snmpsession *sess = GETSESSION(s);
	char rs[MAX_ITEM_LEN];
	memset(rs, 0x0, MAX_ITEM_LEN);
	int ret = snmpget(sess->ip, sess->community, item->oids, rs);
	if( ret != 0 )
	{
		printf("SNMP GET ERROR\n");
		return -1;
	}
	else{
		int len = strlen(rs);
		item->itemhead = (char*)malloc(len+1);
		strcpy(item->itemhead, rs);
		return 0;
	}
}

int clear_table(tableopt* table)
{
	char* handle = table->tablehead;
	char** nextptr = GETNEXTPTR(handle,table->column);
	char* temp = NULL;
	handle = *nextptr;
	while(handle != NULL){
		char** p = NULL;
		for( int i = 0 ;i < table->column; i++){
			p = GETELEMENT(handle, i);	
			if( *p != NULL)
			{
				free(*p);
				*p = NULL;
			}
		}
		nextptr = GETNEXTPTR(handle, table->column);
		temp = handle;
		handle = *nextptr;
		free(temp);
	}
	free(table->tablehead);
	table->tablehead = NULL;
	return 0;
}

int update_table(session_id_t s, tableopt* table)
{
	snmpsession *sess = GETSESSION(s);
	int entries, fields;
	char** data, **dp;
	int ret= 0;
	ret = snmptable(sess->ip, sess->community, table->oids ,&entries, &fields, &data);
	if( ret != 0){
		table->tablehead = NULL;
		return -2;
	}
	else{
		int inited = 0;
		char* handle = NULL;
		if( table->tablehead == NULL )
		{
		  handle = (char*)malloc(table->type_size);
		  char** next = GETNEXTPTR(handle,table->column);
		  *next = NULL;
		  table->tablehead = handle;
		}
		else{
			handle = table->tablehead;
			inited = 1;
		}
		dp =data;
		for(int i = 0; i< entries; i++){
			char** next =GETNEXTPTR(handle, table->column);
			if(inited == 0)
			{
				*next = (char*)malloc(table->type_size);
				char** temp = GETNEXTPTR(*next, table->column);
				*temp = NULL;
			}
			handle = (char*)(*next);
			for(int j= 0; j< table->column; j++){
				char** element = (char**)GETELEMENT(handle,j);
				int k = table->mapper[j];
				if ( dp[k] == NULL ) {//the snmpd's response is not correct
					ret = -1;// error orrcur
					*element = (char*)malloc(10);
					strcpy(*element, "(null)");
				}
				else
				{
					int len = strlen(dp[k]);
					*element = (char*)malloc(len+1);
					strcpy(*element,dp[k]);
				}
			}
			dp = dp + fields;
		}
	}
	free_data(data, entries, fields);
	if( ret == -1) return -1;
	return 0;
}
