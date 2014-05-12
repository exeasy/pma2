#ifndef DDC_H

#define DDC_H


#define UNCONNECTING 		0
#define CONNECTING 			1
#define MAX_METRIC 			2000
#define USING					1
#define NOTUSING				0
#define MAX_INTERFACE       999
#define MAX_ECMP_LIMIT		4

struct ridinfo{
	u32 num;
	u32 rid;
};

struct Netnode {
	u32 nextHop;
	u32 nexthop[MAX_ECMP_LIMIT];
	u32 nexthop_count;
	u32 metric;
	u32 path_cost;
	u32 link_index;
	struct link_info * link;
} /* optional variable list */;

struct Netpath {
	u32 rid;
	u32 nrid;
	u32 connection;
	u32 importance_value;
	u32 distance_metric;
	u32 nexthop_rid; //Frr
	struct Netpath *next;
} /* optional variable list */;


struct paths_of_link{
	struct Netpath * shortpath;
	struct paths_of_link *next;
};

struct link_path_info{
	struct link_info* link;
	struct paths_of_link * allshortpaths;
	int shortpath_count;
};

struct area_info{
	int azid;
	//lsdb info 
	struct lsdb_router * routerlist;
	int router_count;
	struct link_info* lslist;
	int ls_count;
	//lidb info
	int backbone_count;
	int access_count;
	struct ridinfo * nodeinfo;
	struct Netnode * matrix;
	struct Netpath * shortpaths;
	struct link_path_info * linkshadows;
	struct area_info* next;
};
struct global_info{
	int * arealist;
	int area_count;
	struct area_info* azinfo;
};

int path_compute_start();
struct area_info * get_area_by_areaid(u32 areaid);
int record_broken_path_down( struct link_info * link );
float get_net_destory_degree(struct area_info * az);
int is_computed();


int  fah_protect_link(u32 routerid , struct link_info* link);
#endif /* end of include guard: DDC_H */
