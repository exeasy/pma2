#include <utils/list.h>
#include <utils/common.h>
#include <utils/utils.h>
#include <lsdb.h>
#include <private/lsdb_inner.h>

extern unsigned int router_id;
/*  cr_lsdb_init
 *  init a lsdb implement 
 *  */
int cr_lsdb_init(id_t* handle, id_t rid)
{
	struct lsdb* lsdb;
	lsdb = malloc_z(struct lsdb);
	assert(lsdb != NULL);
	cr_init_list_head(&lsdb->routers_head);
	cr_init_list_head(&lsdb->links_state_head);
	*handle = (id_t)lsdb;
	router_id = rid;
	return NO_ERR;
}
/* lsdb_router_add
 *  add router into 
 *  */
int lsdb_router_add(const struct cr_lsdb_router* node, struct router_node** result, struct lsdb* cr_lsdb)
{
	struct router_node *newnode;

	newnode = malloc_z(struct router_node);
	assert(newnode != NULL);
	cr_init_list_head(&newnode->links_state_head);

	newnode->data = *node;

	cr_list_add(&newnode->db_ptrs,&cr_lsdb->routers_head);
	cr_lsdb->routers_count++;

	*result = newnode;
	return NO_ERR;
}

int lsdb_link_state_add(const struct cr_lsdb_link_state* state, struct link_state** result, struct lsdb* cr_lsdb)
{
	struct link_state *newstate;
	struct router_node* rt;
	if((state->key.n_rt_id != 0) && lsdb_router_find(state->key.n_rt_id, &rt, cr_lsdb))
	{
		struct cr_lsdb_router node;
		node.id = state->key.n_rt_id;
		if(lsdb_router_add(&node, &rt, cr_lsdb))
			return N0_DEPENDENCE_ERR;
	}
	if(lsdb_router_find(state->key.rt_id, &rt, cr_lsdb))
	{
		struct cr_lsdb_router node;
		node.id = state->key.rt_id;
		if(lsdb_router_add(&node, &rt, cr_lsdb))
			return N0_DEPENDENCE_ERR;
	}
	newstate = malloc_z(struct link_state);
	assert(newstate != NULL);
	*result = newstate;

	newstate->data = *state;
	newstate->rt = rt;

	cr_list_add(&newstate->db_ptrs,&cr_lsdb->links_state_head);
	cr_lsdb->links_state_count++;
	cr_list_add(&newstate->rt_ptrs,&rt->links_state_head);
	rt->links_state_count++;
	return NO_ERR;
}
int lsdb_router_find(u32 rt_id, struct router_node** result, struct lsdb* cr_lsdb)
{
	struct cr_list_head * ptr;
	struct router_node* entry;

	for(ptr = cr_lsdb->routers_head.next; ptr != &cr_lsdb->routers_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct router_node, db_ptrs);
		if(entry->data.id == rt_id)
		{
			*result = entry;
			return NO_ERR;
		}
	}
	return NOT_FOUND_ERR;
}
int lsdb_link_state_find(const struct link_state_key* key, struct link_state** result, struct lsdb* cr_lsdb)
{
	struct cr_list_head * ptr;
	struct router_node *node;
	struct link_state* entry;

	int r = lsdb_router_find(key->rt_id, &node, cr_lsdb);
	if(r)
		return r;
	for(ptr = node->links_state_head.next; ptr != &node->links_state_head; ptr = ptr->next)
	{
		entry = CR_LIST_ENTRY(ptr, struct link_state, rt_ptrs);
		if((entry->data.key.if_id == key->if_id)
				&&
				(entry->data.key.n_rt_id == key->n_rt_id) && (entry->data.key.n_if_id == key->n_if_id))
		{
			*result = entry;
			return NO_ERR;
		}
	}
	return NOT_FOUND_ERR;
}

/* add cr_lsd_is_connect from cr_hello.c by Macro.Z 2013-01-26 begin */
#define STATE_MASK 0x03

//see whether the link is connected
int cr_lsd_is_connected(const struct cr_lsdb_link_state* const lsa, id_t lsdb_handle)
{
	struct link_state_key n_key = {.rt_id = lsa->key.n_rt_id, .if_id = lsa->key.n_if_id,
		.n_rt_id = lsa->key.rt_id, .n_if_id = lsa->key.if_id};

	struct cr_lsdb_link_state n_lsa;
	if(cr_lsdb_link_state_find(&n_key, &n_lsa, lsdb_handle) != NO_ERR)
		return 0;

	if(((lsa->state & STATE_MASK) == STATE_MASK) && ((n_lsa.state & STATE_MASK) == STATE_MASK))
		return 1;
	else
		return 0;
}
