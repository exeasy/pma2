#ifndef DBM_LSDB_H

#define DBM_LSDB_H

struct lsdb_router{
	u32 rid;
	struct lsdb_router* next;
};

struct link_info{
	u32 areaid;
	u32 rid;
	u32 nrid;
	u32 ifid;
	u32 nifid;
	u32 metric;
	u32 status;
	u32 seq;
	struct timeval age;
};
int insert_link_state( struct link_info * link);

int update_link_state( struct link_info * link);

int* get_areaes(int *count);

int get_routers_count(int areaid);

struct lsdb_router* get_routers(int areaid);

struct link_info* get_link_states(u32 areaid, int *count);

struct link_info* get_link_states_of_router(u32 areaid,u32 routerid , int *count);

int query_link_state_status(u32 areaid, u32 routerid ,u32 neighbor_routerid);

int query_link_state_metric(u32 areaid, u32 routerid ,u32 neighbor_routerid);

int query_link_state_details(u32 areaid, u32 routerid ,u32 neighbor_routerid, struct link_info* link);
char* format_lsdb_to_xml(u32 areaid, u32 routerid,int type);
char* format_all_area_lsdb_to_xml(int routerid);

#endif /* end of include guard: DBM_LSDB_H */
