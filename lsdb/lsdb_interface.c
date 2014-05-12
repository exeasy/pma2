#include <utils/list.h>
#include <utils/common.h>
#include <utils/utils.h>
#include <lsdb.h>
#include <lsdb_sender.h>
#include <private/lsdb_inner.h>
#include <lsd/lsd_if.h>

unsigned int router_id;
extern int update_register_tag;


int cr_lsdb_router_add(const struct cr_lsdb_router* router, id_t lsdb_handle)
{
	struct router_node* p;
	assert(router->id != 0);
	assert(lsdb_router_find(router->id, &p, (struct lsdb*)lsdb_handle) == NOT_FOUND_ERR);
	return lsdb_router_add(router, &p, (struct lsdb*)lsdb_handle);
}
int cr_lsdb_router_find(struct cr_lsdb_router* router, id_t lsdb_handle)
{
	int r;
	struct router_node* p;
	if(router->id == 0) return NOT_FOUND_ERR;
	r = lsdb_router_find(router->id, &p, (struct lsdb*)lsdb_handle);
	if(!r)
		*router = p->data;
	return r;
}
int cr_lsdb_router_update(const struct cr_lsdb_router* router, id_t lsdb_handle)
{
	int r;
	struct router_node* p;
	assert(router->id != 0);
	r = lsdb_router_find(router->id, &p, (struct lsdb*)lsdb_handle);
	if(!r)
		p->data = *router;
	return r;
}
int cr_lsdb_link_state_add(const struct cr_lsdb_link_state* link, id_t lsdb_handle)
{
	struct link_state* result;
	assert(!(link->key.rt_id == 0));
	assert(NOT_FOUND_ERR == lsdb_link_state_find(&link->key, &result, (struct lsdb*)lsdb_handle));
	/*modify by Macro.Z
	 * add area ,pack the lsdb,then send it
	 */
	int area = 0,len = 0;
	area = get_area_id_by_lsdb_handle(lsdb_handle);
	lsdb_Msg* msg =  lsdb_msg_create(area,link);
	char* pkt = create_lsdb_packet(ADDLSA,msg,&len);
	send_packet((Packet_header*)pkt);
	if(msg!=NULL)
		{
		free(msg);msg= NULL;
		}
	return lsdb_link_state_add(link, &result, (struct lsdb*)lsdb_handle);
}

int cr_lsdb_link_state_find(const struct link_state_key* key, struct cr_lsdb_link_state* link, id_t lsdb_handle)
{
	int r;
	struct link_state* result;
	if(key->rt_id == 0) return NOT_FOUND_ERR;

	r = lsdb_link_state_find(key, &result, (struct lsdb*)lsdb_handle);
	if(!r)
	{
		// *link = result->data;
		memcpy(link,&(result->data),sizeof(struct cr_lsdb_link_state));
		link->shadow	= result->data.shadow;
	}
	return r;
}

int cr_lsdb_neighbour_info_find(struct link_state_key* key, id_t lsdb_handle)
{
	struct cr_list_head * ptr;
	struct router_node *node;
	struct link_state* entry;
	struct lsdb* cr_lsdb;

	cr_lsdb = (struct lsdb*)lsdb_handle;
	int r = lsdb_router_find(key->rt_id, &node, cr_lsdb);
	if(r)
		return r;
	for(ptr = node->links_state_head.next; ptr != &node->links_state_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct link_state, rt_ptrs);
		if(entry->data.key.if_id == key->if_id)
		{
			key->n_rt_id = entry->data.key.n_rt_id;
			key->n_if_id = entry->data.key.n_if_id;
			return NO_ERR;
		}
	}
	return NOT_FOUND_ERR;
}

int cr_lsdb_ethid_info_find_by_nrt_id(struct link_state_key* key, id_t lsdb_handle)
{
	struct cr_list_head * ptr;
	struct router_node *node;
	struct link_state* entry;
	struct lsdb* cr_lsdb;

	cr_lsdb = (struct lsdb*)lsdb_handle;
	int r = lsdb_router_find(key->rt_id, &node, cr_lsdb);
	if(r)
		return r;
	for(ptr = node->links_state_head.next; ptr != &node->links_state_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct link_state, rt_ptrs);
		if(entry->data.key.n_rt_id == key->n_rt_id)
		{
			key->if_id = entry->data.key.if_id;
			key->n_if_id = entry->data.key.n_if_id;
			return NO_ERR;
		}
	}
	return NOT_FOUND_ERR;
}



int cr_lsdb_link_state_update(const struct cr_lsdb_link_state* link, id_t lsdb_handle)
{
	int r;
	struct link_state* result;
	assert(!(link->key.rt_id == 0));

	/*modify by Macro.Z
		 * add area ,pack the lsdb,then send it
		 */
		 //add for test
		// show_lsdb();
		
	r = lsdb_link_state_find(&link->key, &result, (struct lsdb*)lsdb_handle);
	if(!r)
	{
		int area = 0,len = 0;
		area = get_area_id_by_lsdb_handle(lsdb_handle);
		lsdb_Msg* msg =  lsdb_msg_create(area,link);
		char* pkt = create_lsdb_packet(UPDATELSA,msg,&len);
		send_packet((Packet_header*)pkt);
		if(msg!=NULL)
		{
			free(msg);msg= NULL;
		}
		//result->data = *link;
		//
		//printf("\n update link state %p\n",link->shadow);
	}
	//r = lsdb_link_state_find(&link->key, &result, (struct lsdb*)lsdb_handle);
	//if(!r)
	//{
	//	//result->data = *link;
	//	//printf("\n update link state %p\n",link->shadow);
	//	int original_state = result->data.state;
	//	int now_state = link->state;
	//	int r_id = link->key.rt_id;
	//	if(original_state!=now_state)
	//	{
	//		if(r_id ==router_id )//self linkstate changed
	//		{	
	//		//	update_register_tag =1;//LInk state changed
	//			if(now_state == 3)//
	//			{
	//				int if_id = link->key.if_id;
	//				//fah_set_default_timer(if_id);
	//			}
	//			else if(now_state == 1)
	//			{
	//				//cpi_policy_get(link);
	//				//fah_reprotect(link);
	//			}
	//		}
	//		else//other linkstate changed
	//		{
	//			//update_register_tag =1;//LInk state changed
	//			if(now_state == 1&&original_state == 3)
	//			{
	//				struct link_state_key link_key_reverse;
	//				link_key_reverse.rt_id = link->key.n_rt_id;
	//				link_key_reverse.if_id = link->key.n_if_id;
	//				link_key_reverse.n_rt_id = link->key.rt_id;
	//				link_key_reverse.n_if_id = link->key.if_id;
	//				struct cr_lsdb_link_state result_reverse;
	//				int ret_rev = cr_lsdb_link_state_find(&link_key_reverse, &result_reverse, lsdb_handle);
	//				if(ret_rev == NOT_FOUND_ERR||((result_reverse.state!=0)&&(result_reverse.state!=1)))
	//				{
	//					struct cr_lsdb_link_state cr_link;
	//					cr_link.key = link_key_reverse;

	//				//	fah_reprotect(&cr_link);
	//				}
	//			}

	//		}
	//	}
	memcpy(&(result->data),link,sizeof(struct cr_lsdb_link_state));
	return r;
}
inline int cr_lsdb_get_routers_count(id_t lsdb_handle)
{
	return ((struct lsdb*)lsdb_handle)->routers_count;
}
inline int cr_lsdb_get_link_states_count(id_t lsdb_handle)
{
	return ((struct lsdb*)lsdb_handle)->links_state_count;
}

