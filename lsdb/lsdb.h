#ifndef LSDB_H_
#define LSDB_H_

#define MAX_ADJ 10


struct link_state_key{
	u32 rt_id;
	u32 if_id;

	u32 n_rt_id;
	u32 n_if_id;
};

struct link_state_adv{
	struct link_state_key key;

	struct in_addr addr;
	u32 metric;
	u8 state;

	u32 seq;
};

struct cr_lsdb_router
{
	u32 id;
	struct cr_lsdb_router* next;
};
struct cr_lsdb_link_state{
	struct link_state_key key;

	struct in_addr addr;
	u32 metric;
	u8 state;
	void * shadow;
	u32 seq;
	struct timeval age;
	struct cr_lsdb_link_state* next;
};

int cr_lsdb_init(id_t* handle, id_t rid);

inline int cr_lsdb_get_routers_count(id_t lsdb_handle);//tested
inline int cr_lsdb_get_link_states_count(id_t lsdb_handle);//tested

int cr_lsdb_router_add(const struct cr_lsdb_router* router, id_t lsdb_handle);//tested
int cr_lsdb_router_update(const struct cr_lsdb_router* router, id_t lsdb_handle_handle);//tested
int cr_lsdb_router_find(struct cr_lsdb_router* node, id_t lsdb_handle);//tested

int cr_lsdb_link_state_add(const struct cr_lsdb_link_state* link, id_t lsdb_handle);//tested
int cr_lsdb_link_state_update(const struct cr_lsdb_link_state* link, id_t lsdb_handle);//tested
int cr_lsdb_link_state_find(const struct link_state_key* key, struct cr_lsdb_link_state* link, id_t lsdb_handle);//tested
int cr_lsdb_neighbour_info_find(struct link_state_key* key, id_t lsdb_handle);//tested
int cr_lsdb_ethid_info_find_by_nrt_id(struct link_state_key* key, id_t lsdb_handle);
//routers traverse
typedef int (*routers_visitor)(struct cr_lsdb_router* rt, void* args);
int cr_lsdb_routers_accept(routers_visitor v, void* args, id_t lsdb_handle);
int cr_lsdb_get_routers(struct cr_lsdb_router** head, id_t lsdb_handle);

//access links traverse
typedef int (*links_state_visitor)(
		struct cr_lsdb_router* rt, struct cr_lsdb_link_state* state, void* args);
int cr_lsdb_links_state_accept(links_state_visitor v, void* args, id_t lsdb_handle);
int cr_lsdb_backbone_links_state_accept(links_state_visitor v, void* args, id_t lsdb_handle);
int cr_lsdb_links_state_accept_by_router(
		u32 rt, links_state_visitor v, void* args, id_t lsdb_handle);
int cr_lsdb_access_links_state_accept_by_router(
		u32 rt_id, links_state_visitor v, void* args, id_t lsdb_handle);
int cr_lsdb_backbone_links_state_accept_by_router(
		u32 rt_id, links_state_visitor v, void* args, id_t lsdb_handle);
int cr_lsdb_get_links_state(struct cr_lsdb_link_state** head, id_t lsdb_handle);
int cr_lsdb_get_backbone_links_state(struct cr_lsdb_link_state** head, id_t lsdb_handle);
int cr_lsdb_get_links_state_by_router(struct cr_lsdb_link_state** head, u32 rt_id, id_t lsdb_handle);
int cr_lsdb_get_access_links_state_by_router(struct cr_lsdb_link_state** head, u32 rt_id, id_t lsdb_handle);
int cr_lsdb_get_backbone_links_state_by_router(struct cr_lsdb_link_state** head, u32 rt_id, id_t lsdb_handle);
int cr_lsdb_find_adjacent_router(id_t lsdb_handle, u32 rt_id, int array[MAX_ADJ]);

int cr_lsdb_free_links_state_list(struct cr_lsdb_link_state* head);
int cr_lsdb_free_router_list(struct cr_lsdb_router* head);

//see whether the link is connected
int cr_lsd_is_connected(const struct cr_lsdb_link_state* const lsa, id_t lsdb_handle);


#endif /* LSDB_H_ */

