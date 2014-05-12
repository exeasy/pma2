#ifndef LSDB_INNER_H_
#define LSDB_INNER_H_

struct router_node
{
	u32 links_state_count;

	struct cr_lsdb_router data;

	struct cr_list_head links_state_head;
	struct cr_list_head db_ptrs;
};
struct link_state
{
	struct cr_lsdb_link_state data;
	struct router_node* rt;

	struct cr_list_head db_ptrs;
	struct cr_list_head rt_ptrs;
};
struct lsdb
{
	u32 routers_count;
	u32 links_state_count;

	struct cr_list_head routers_head;
	struct cr_list_head links_state_head;
};
int lsdb_router_add(const struct cr_lsdb_router* node, struct router_node** result, struct lsdb* cr_lsdb);
int lsdb_router_find(u32 id, struct router_node** result, struct lsdb* cr_lsdb);
int lsdb_link_state_add(const struct cr_lsdb_link_state* link, struct link_state** result, struct lsdb* cr_lsdb);
int lsdb_link_state_find(const struct link_state_key* key, struct link_state** result, struct lsdb* cr_lsdb);

#endif /* LSDB_INNER_H_ */

