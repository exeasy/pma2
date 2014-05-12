#include <utils/list.h>
#include <utils/common.h>
#include <utils/utils.h>
#include <lsdb.h>
#include <private/lsdb_inner.h>
struct cr_lsdb_router test;
#define LINK_MASK 0x03
int cr_lsdb_routers_accept(routers_visitor v, void* args, id_t lsdb_handle)
{
	struct lsdb* db = (struct lsdb*)lsdb_handle;
	struct router_node* entry;
	struct cr_list_head* ptr;
	for(ptr = db->routers_head.next; ptr != &db->routers_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct router_node, db_ptrs);
		int r = (*v)(&entry->data, args);
		if(r)
			return r;
	}
	return NO_ERR;
}
int cr_lsdb_links_state_accept(links_state_visitor v, void* args, id_t lsdb_handle)
{
	struct lsdb* db = (struct lsdb*)lsdb_handle;
	struct link_state* entry;
	struct cr_list_head* ptr;
	for(ptr = db->links_state_head.next; ptr != &db->links_state_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct link_state, db_ptrs);

		int r = (*v)(&entry->rt->data, &entry->data, args);
		if(r)
			return r;
	}
	return NO_ERR;
}

int cr_lsdb_backbone_links_state_accept(links_state_visitor v, void* args, id_t lsdb_handle)
{
	struct lsdb* db = (struct lsdb*)lsdb_handle;
	struct link_state* entry;
	struct cr_list_head* ptr;
	for(ptr = db->links_state_head.next; ptr != &db->links_state_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct link_state, db_ptrs);
		if(entry->data.key.n_rt_id != 0)
		{
			int r = (*v)(&entry->rt->data,&entry->data, args);
			if(r)
				return r;
		}
	}
	return NO_ERR;
}

int cr_lsdb_links_state_accept_by_router(
		u32 rt_id, links_state_visitor v, void* args, id_t lsdb_handle)
{
	int r;
	struct router_node* rt;
	r = lsdb_router_find(rt_id, &rt, (struct lsdb*)lsdb_handle);
	if(r)
		return r;
	//	lsdb_printf("%d", rt_id);
	struct link_state* entry;
	struct cr_list_head* ptr;
	for(ptr = rt->links_state_head.next; ptr != &rt->links_state_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct link_state, rt_ptrs);
		//		lsdb_printf("%d  rt_id", rt->data.id);
		//		lsdb_printf("%d  rt_id", entry->data.key.rt_id);
		int r = (*v)(&rt->data,&entry->data, args);
		if(r)
			return r;
	}
	return NO_ERR;
}

int cr_lsdb_access_links_state_accept_by_router(
		u32 rt_id, links_state_visitor v, void* args, id_t lsdb_handle)
{
	int r;
	struct router_node* rt;
	r = lsdb_router_find(rt_id, &rt, (struct lsdb*)lsdb_handle);
	if(r)
		return r;
	//	lsdb_printf("%d", rt_id);
	struct link_state* entry;
	struct cr_list_head* ptr;
	for(ptr = rt->links_state_head.next; ptr != &rt->links_state_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct link_state, rt_ptrs);
		//		lsdb_printf("%d  rt_id", rt->data.id);
		//		lsdb_printf("%d  rt_id", entry->data.key.rt_id);
		if(entry->data.key.n_rt_id == 0)
		{
			int r = (*v)(&rt->data,&entry->data, args);
			if(r)
				return r;
		}
	}
	return NO_ERR;
}

int cr_lsdb_backbone_links_state_accept_by_router(
		u32 rt_id, links_state_visitor v, void* args, id_t lsdb_handle)
{
	int r;
	struct router_node* rt;
	r = lsdb_router_find(rt_id, &rt, (struct lsdb*)lsdb_handle);
	if(r)
		return r;
	//	lsdb_printf("%d", rt_id);
	struct link_state* entry;
	struct cr_list_head* ptr;
	for(ptr = rt->links_state_head.next; ptr != &rt->links_state_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct link_state, rt_ptrs);
		//		lsdb_printf("%d  rt_id", rt->data.id);
		//		lsdb_printf("%d  rt_id", entry->data.key.rt_id);
		if(entry->data.key.n_rt_id != 0)
		{
			int r = (*v)(&rt->data,&entry->data, args);
			if(r)
				return r;
		}
	}
	return NO_ERR;
}

int add_router_into_list(struct cr_lsdb_router* rt, void* args)
{
	struct cr_lsdb_router* new = malloc_z(struct cr_lsdb_router);
	if(!new)
		return NO_MEM_ERR;
	*new = *rt;
	new->next = (*(struct cr_lsdb_router**)args);
	(*(struct cr_lsdb_router**)args) = new;

	return NO_ERR;
}
int cr_lsdb_get_routers(struct cr_lsdb_router** head, id_t lsdb_handle)
{
	*head = NULL;
	return cr_lsdb_routers_accept(add_router_into_list, head, lsdb_handle);
}

int add_link_state_into_list(struct cr_lsdb_router* rt, struct cr_lsdb_link_state* state, void* args)
{
	struct cr_lsdb_link_state* new = malloc_z(struct cr_lsdb_link_state);
	if(!new)
		return NO_MEM_ERR;
	*new = *state;
	new->next = (*(struct cr_lsdb_link_state**)args);
	(*(struct cr_lsdb_link_state**)args) = new;

	return NO_ERR;
}
int cr_lsdb_get_links_state(struct cr_lsdb_link_state** head, id_t lsdb_handle)
{
	*head = NULL;
	return cr_lsdb_links_state_accept(add_link_state_into_list, head, lsdb_handle);
}
int cr_lsdb_get_backbone_links_state(struct cr_lsdb_link_state** head, id_t lsdb_handle)
{
	*head = NULL;
	return cr_lsdb_backbone_links_state_accept(add_link_state_into_list, head, lsdb_handle);
}
int cr_lsdb_get_links_state_by_router(struct cr_lsdb_link_state** head, u32 rt_id, id_t lsdb_handle)
{
	*head = NULL;
	return cr_lsdb_links_state_accept_by_router(rt_id, add_link_state_into_list, head, lsdb_handle);
}

int cr_lsdb_get_access_links_state_by_router(struct cr_lsdb_link_state** head, u32 rt_id, id_t lsdb_handle)
{
	*head = NULL;
	return cr_lsdb_access_links_state_accept_by_router(rt_id, add_link_state_into_list, head, lsdb_handle);
}

int cr_lsdb_get_backbone_links_state_by_router(struct cr_lsdb_link_state** head, u32 rt_id, id_t lsdb_handle)
{
	*head = NULL;
	return cr_lsdb_backbone_links_state_accept_by_router(rt_id, add_link_state_into_list, head, lsdb_handle);
}

int cr_lsdb_free_links_state_list(struct cr_lsdb_link_state* head)
{
	struct cr_lsdb_link_state *ptr1, *ptr2;
	ptr1 = head;
	while(ptr1)
	{
		ptr2 = ptr1->next;
		free(ptr1);
		ptr1 = ptr2;
	}
	return NO_ERR;
}

int cr_lsdb_free_router_list(struct cr_lsdb_router* head)
{
	struct cr_lsdb_router* ptr1, *ptr2;
	ptr1 = head;
	while(ptr1)
	{
		ptr2 = ptr1->next;
		free(ptr1);
		ptr1 = ptr2;
	}
	return NO_ERR;
}

int cr_lsdb_find_adjacent_router(id_t lsdb_handle, u32 rt_id, int array[MAX_ADJ])
{
	struct cr_lsdb_link_state* head = NULL;
	struct cr_lsdb_link_state* ptr = NULL;
	cr_lsdb_get_backbone_links_state_by_router(&head, rt_id, lsdb_handle);

	int i = 0;
	for(ptr = head; ptr; ptr = ptr->next)
	{
		if((ptr->state&LINK_MASK) == LINK_MASK)
		{
			array[i] = ptr->key.rt_id;
			i++;
		}
	}

	cr_lsdb_free_links_state_list(head);

	array[i] = -1;

	return NO_ERR	;
}

