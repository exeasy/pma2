#include <utils/common.h>
#include <utils/utils.h>
#include <lib/linklist.h>
#include "lib/if.h"
#include <lib/prefix.h>
#include <lib/thread.h>
#include <router/interface.h>
#include <router/router.h>
#include "lsd_cmn.h"
#include "lsd_event.h"
#include <lib/event.h>
#include "lsd_if.h"

declare_event_queue(access_queue);
declare_event_queue(backbone_queue);

extern struct router localrouter;
extern struct thread_master * master;
struct lsd_router g_lsd_router;

int eth_state_update(struct common_eth* eth)
{
	if(eth == NULL){
		return -1;
	}
	if( eth->state == ETH_UP )
    {
		thread_add_event(master,interface_up,eth ,0);
    }
	else
    {
		thread_add_event(master,interface_down,eth,0);
    }
	return 0;
}

int interface_up(struct thread* thread){
	struct common_eth* eth = NULL;
	struct backbone_eth* bb_eth = NULL;
	struct access_eth* access_eth = NULL;
		eth = (struct common_eth *) THREAD_ARG (thread);
		assert(eth != NULL);
		if(eth->type == ACCESS_ETH )
		{
            access_eth = (struct access_eth*)eth;
			printf("NIC \"%d\" up.type: ACCESS\n", access_eth->_ifid );
			//access_eth->eth_state = ETH_UP;


			struct event_handler_item* item;
			for(item = access_queue.next; item; item = item->next)
				((access_eth_state_handler)(item->handler))(access_eth);

		}
		else if( eth->type == BACKBONE_ETH )
		{
            bb_eth = (struct backbone_eth*)eth;
			printf("NIC \"%d\" up.type: BACKBONE\n", bb_eth->_ifid);

			struct event_handler_item* item;
			for(item = backbone_queue.next; item; item = item->next)
            {
				((backbone_eth_state_handler)(item->handler))(bb_eth);
            }

		}
		return 0;
}

int interface_down (struct thread* thread){
	struct common_eth* eth = NULL;
	struct backbone_eth* bb_eth = NULL;
	struct access_eth* access_eth = NULL;

	eth = (struct common_eth *) THREAD_ARG (thread);
	assert(eth != NULL);
	if( ACCESS_ETH == eth->type )
		{
			printf("NIC \"%d\" down.type: ACCESS\n", eth->ifid);
            access_eth = (struct access_eth*)eth;
			struct event_handler_item* item;
			for(item = access_queue.next; item; item = item->next)
				((access_eth_state_handler)(item->handler))(access_eth);

		}
		else if( BACKBONE_ETH == eth->type )
		{
			printf("NIC \"%d\" down.type: BACKBONE\n", eth->ifid);
            bb_eth = (struct backbone_eth*)eth;
			struct event_handler_item* item;
			for(item = backbone_queue.next; item; item = item->next)
				((backbone_eth_state_handler)(item->handler))(bb_eth);

		}
		return 0;
}

int show_if_info(struct common_eth * eth)
{
    if ( eth->type == BACKBONE )
    {
        struct backbone_eth * backbone = (struct backbone_eth*)eth;
        char * name = backbone->interface->name;
        int rid = backbone->neighbor_pma.rid;
        char router_addr[24];
        char pma_ctl_addr[24];
        inet_ntop(AF_INET, &backbone->neighbor_pma.router_addr, router_addr, 24);
        inet_ntop(AF_INET, &backbone->neighbor_pma.pma_ctl_addr, pma_ctl_addr, 24);
        DEBUG(INFO, "%s %d %s %s\n", name, rid, router_addr, pma_ctl_addr);
    }
}

int lsd_iflist_init()
{
    if( localrouter.inited == 0 )
    {
        printf( "local router havent inited yet.\n" );
        return -1;
    }
    int t_if = 0;
    struct listnode *node;
    void *data;
    struct rinterface *face;
    LIST_LOOP(localrouter.iflist, data, node)
    {
        face = (struct rinterface*)data;
        if( face->type == BACKBONE )
        {
            struct backbone_eth * backbone = face->vinterface; 
            int new_create = 0;
            if ( backbone  == NULL )
            {
                backbone = malloc_z(struct backbone_eth);
                new_create = 1;
            }
            
            backbone->interface = face;
            backbone->_ifid = face->ctl_id;
            backbone->_state = face->op_state;
            backbone->_type = BACKBONE_ETH;
            if( face->neighbor != NULL)
            {
				backbone->neighbor_pma.device_id = face->neighbor->devid;
                backbone->neighbor_pma.rid = face->neighbor->rid;
                backbone->neighbor_pma.router_addr = face->neighbor->router_addr;
                backbone->neighbor_pma.pma_ctl_addr = face->neighbor->agent_addr;
            }

            struct proto_info *proto = (struct proto_info*)face->proto_info;
            //if ( proto->proto_type == 0 ) // ospf
           // {
			int areaid = 0;
				if( proto->proto_type == ROUTER_BGP )
				{
					areaid = 0;
				}
				else if ( proto->proto_type == ROUTER_OSPF )
				{
					areaid = proto->u.areaid;
				}
                struct autonomous_zone* az = find_autonomous_zone(areaid);
                if( NULL == az )
                {
                    az = malloc_z(struct autonomous_zone);
                    az->az_id = areaid;
					az->device_id = localrouter.agentid;
                    az->r_id = localrouter.routerid;
					cr_lsdb_init(&az->lsdb, az->r_id);
                    autonomous_zone_add(az);
                }
            //}
            printf( "Area %d IF %d\n", areaid, backbone->_ifid );
            face->vinterface = backbone;
            if ( new_create ) backbone_eth_add(az, backbone);
            show_if_info((struct common_eth*)backbone);
            eth_state_update((struct common_eth*)backbone);
        }
        else if (face->type == ACCESS)
        {
            struct access_eth * access = face->vinterface;
            if ( access == NULL)
            {
                access = malloc_z(struct access_eth);
                access_eth_add(access);
            }
            access->interface = face;
            access->_ifid = face->ctl_id;
            access->_type = ACCESS_ETH;
            access->_state = face->op_state;
            
            face->vinterface = access;
            eth_state_update((struct common_eth*)access);
        }
    }
}

// find the specific backbone in area
struct backbone_eth* get_backbone_by_neighbor_id(struct autonomous_zone* az, int id){
    struct backbone_eth * eth = az->backbones;
    while(eth)
    {
            if ( eth->neighbor_pma.device_id == id )
                return eth;
        eth = eth->next;
    }
    return NULL;
}

struct autonomous_zone* find_autonomous_zone(int area_id){
	struct autonomous_zone* az = g_lsd_router.azs;
	while(az!=NULL){
		if(az->az_id == area_id)
			return az;
		az = az->next;
		}
	return NULL;
}
// ***********
struct autonomous_zone* get_autonomous_zone_by_id(u32 area_id)
{
	struct autonomous_zone* az = g_lsd_router.azs;
	while(az)
	{
		if(az->az_id == area_id)
			return az;
		az = az->next;
	}
	return NULL;
}
u32 get_area_id_by_lsdb_handle(id_t lsdb_handle)
{
	struct autonomous_zone* az = g_lsd_router.azs;
	while(az)
	{
		if(az->lsdb == lsdb_handle)
		{
			return az->az_id;
		}
		az = az->next;
	}
	return NOT_FOUND_ERR;
}
int * get_all_areas(int * count)
{
	if (NULL ==  g_lsd_router.azs || count == NULL )
		{
			return NULL;
		}
	struct autonomous_zone* az = g_lsd_router.azs;
	*count = 0;
	int *azs = (int*)malloc(1024);
	while(az)
	{
		azs[*count] = az->az_id;
		az = az->next;
		(*count)++;
	}
	return azs;
}
//append functions
void backbone_eth_add(struct autonomous_zone* azone,struct backbone_eth *backbone_eth)
{
	assert(azone != NULL);
	assert(backbone_eth != NULL);

	backbone_eth->az = azone;
	backbone_eth->next = azone->backbones;
	azone->backbones = backbone_eth;
}
void access_eth_add(struct access_eth *access_eth)
{
	assert(access_eth != NULL);

	access_eth->next = g_lsd_router.accesses;
	g_lsd_router.accesses = access_eth;
}
void autonomous_zone_add(struct autonomous_zone *az)
{
	assert(az != NULL);

	az->next = g_lsd_router.azs;
	g_lsd_router.azs = az;
}

int get_metric_of_backbone( struct backbone_eth * eth)
{
	assert(eth != NULL);
	return eth->interface->proto_info->cost;
}

int get_metric_of_access( struct access_eth * eth)
{
	assert(eth != NULL);
	return eth->interface->proto_info->cost;
}

int  get_address_of_access( struct access_eth* eth, struct in_addr* addr)
{
	assert( eth != NULL);
	assert( addr != NULL);
	struct list* addrlist = list_new();
	int total = 0;
	get_address_of_interface(eth->interface, &addrlist, &total);
	if ( total >=1 )
	{
		struct connected *con_ip = (struct connected *)(addrlist->head->data);
		*addr = con_ip->address->u.prefix4; 
		list_delete_all_node(addrlist);
		list_free(addrlist);
		return 0;
	}
	else{
		list_delete_all_node(addrlist);
		list_free(addrlist);
		return -1;
	}
}
int  get_address_of_backbone( struct backbone_eth* eth, struct in_addr* addr)
{
	assert( eth != NULL);
	assert( addr != NULL);
	struct list* addrlist = list_new();
	int total = 0;
	get_address_of_interface(eth->interface, &addrlist, &total);
	if ( total >=1 )
	{
		struct connected *con_ip = (struct connected *)(addrlist->head->data);
		*addr = con_ip->address->u.prefix4; 
		list_delete_all_node(addrlist);
		list_free(addrlist);
		return 0;
	}
	else
	{
		list_delete_all_node(addrlist);
		list_free(addrlist);
		return -1;
	}
}
