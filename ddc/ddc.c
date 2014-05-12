#include <utils/utils.h>
#include <utils/common.h>
#include "ddc.h"
#include <db/dbm_lsdb.h>
#include <server/dbm.h>

int already_computed = 0;

pthread_mutex_t info_mutex;

struct global_info * info_handle;

int get_index_by_rid(u32 rid, struct area_info * area);

int is_computed()
{
	return already_computed;
}

int init_lsdb_info()
{
	if ( info_handle == NULL )
		info_handle = malloc_z(struct global_info);
	memset( info_handle , 0x00 , sizeof(struct global_info));

	//get the area info
	info_handle->arealist = get_areaes(&info_handle->area_count);
	
	// get each info of area
	int count  = info_handle->area_count;
	info_handle->azinfo = (struct area_info*)malloc(sizeof(struct area_info)*count);

	int i = 0;
	for ( ;  i < count ; i++ )
	{
		struct area_info * az = &info_handle->azinfo[i];
		az->azid = info_handle->arealist[i];
		az->router_count = get_routers_count(az->azid);
		az->routerlist = get_routers(az->azid);
		az->lslist = get_link_states(az->azid, &az->ls_count);
	//	az->next = az+1;
	}
}
// lidb must init after lsdb init
int init_lidb_info()
{
	if ( info_handle == NULL )
		return -1;

	int area_count  = info_handle->area_count;
	int i = 0;
	for( ; i < area_count ; i++ )
	{
		struct area_info * az = &info_handle->azinfo[i];
		u32 azid = az->azid;
		int router_count = az->router_count;
		struct lsdb_router* routerlist = az->routerlist;
		//get_routers sort the routerid in asc order
		az->nodeinfo = (struct ridinfo*)malloc(sizeof(struct ridinfo)* router_count);	
		int j = 0;
		for ( ; j < router_count ;j++ )
		{
			az->nodeinfo[j].num = j;
			az->nodeinfo[j].rid = routerlist->rid;
			routerlist = routerlist->next;
		}

		int ls_count = az->ls_count;
		struct link_info * lslist = az->lslist; 
		az->matrix = (struct Netnode*)malloc( router_count*router_count*sizeof(struct Netnode));
		int m = 0 , n = 0;
		for (m = 0 ; m < router_count ; m++ )
			for (n = 0 ; n < router_count ; n++ )
			{
				struct Netnode * node = az->matrix + m*router_count + n;
				node->link = NULL;
				node->link_index = -1;// -1 present not exist, attention! cant use 0 here.
				node->metric = MAX_METRIC;
				node->nexthop_count = 0;
				node->path_cost = 0;
				memset(node->nexthop, 0x00,  MAX_ECMP_LIMIT*sizeof(int));
			}

		az->linkshadows = (struct link_path_info*)malloc( ls_count * sizeof(struct link_path_info));
		az->access_count =0 ;
		az->backbone_count =0;
		for ( j = 0 ; j < ls_count ; j++ )
		{
			// dont forget the access link
			if ( lslist[j].nrid == 0 && lslist[j].nifid == 0)
			{
				az->linkshadows[j].link = &lslist[j];
				az->linkshadows[j].allshortpaths = NULL;
				az->linkshadows[j].shortpath_count = 0;
				az->access_count++;
				continue;
			}
		//	if ( lslist[j].status == 0 ) continue;
			int x = get_index_by_rid( lslist[j].rid, az);
			int y = get_index_by_rid( lslist[j].nrid, az);
			struct Netnode *node = az->matrix + x*router_count + y;
			node->link = &lslist[j];
			node->link_index = j;
			node->metric = lslist[j].metric;
			node->path_cost = node->metric;
			if ( node->metric < MAX_METRIC )
			{
				node->nexthop_count = 1;
				node->nexthop[0] = y;
			}

			az->linkshadows[j].link = &lslist[j];
			az->linkshadows[j].allshortpaths =  NULL;
			az->linkshadows[j].shortpath_count = 0;
			az->backbone_count++;
			
			az->shortpaths = NULL;
			//printf("LS:%d %d %d\n",lslist[j].rid, lslist[j].nrid, lslist[j].metric);
			//printf("%d %d %d %d\n", x, y, node->metric, node->nexthop[0]);
		}
	}
	
}
int free_info_handle()
{
	if ( info_handle == NULL) 
		return 0;
	free(info_handle->arealist);
	int i = 0 ;
	for ( ; i < info_handle->area_count ; i++ )
	{
		free(info_handle->azinfo[i].lslist);
		free(info_handle->azinfo[i].routerlist);
		free(info_handle->azinfo[i].matrix);
		free(info_handle->azinfo[i].nodeinfo);
		int j = 0;
		for ( j ; j < info_handle->azinfo[i].ls_count; j ++ )
		{
			struct link_path_info * lpinfo = 
			&info_handle->azinfo[i].linkshadows[j];
			struct paths_of_link * path = lpinfo->allshortpaths;
			if ( path == NULL) // access interface ,ignore it
				continue;
			int n = 0;
			for ( n ; n < lpinfo->shortpath_count; n++ )
			{
				struct paths_of_link *tmp_path = path;
				path = path->next;
				free(tmp_path);
			}
		}
		free(info_handle->azinfo[i].linkshadows);
		struct Netpath* temp = info_handle->azinfo[i].shortpaths;
		while(temp)
		{
			struct Netpath* tmp = temp;
			temp = temp->next;
			free(tmp);
		}
	}
	free(info_handle->azinfo);
	return 0;
}

struct  Netnode* create_ddc_matrix(int router_count)
{
	
}

struct area_info * get_area_by_areaid(u32 areaid)
{
	int i = 0;
	for ( ;i < info_handle->area_count ; i++ )
	{
		if ( info_handle->azinfo[i].azid == areaid )
			return &info_handle->azinfo[i];
	}
	return NULL;
}

int get_index_by_rid(u32 rid, struct area_info * area)
{
	if(area == NULL ) return -1;
	struct ridinfo * nlist = area->nodeinfo;
	if ( nlist == NULL) return -1;
	int i = 0; 
	for ( ; i < area->router_count ; i++)
	{
		if ( nlist[i].rid == rid )
			return nlist[i].num;
	}
	return -1;
}

int get_rid_by_index(u32 index, struct area_info *area)
{
	if(area == NULL ) return -1;
	struct ridinfo * nlist = area->nodeinfo;
	if ( nlist == NULL) return -1;
	return nlist[index].rid;
}


/* modify for ecmp consider by Macro.Z 2013-01-29 begin */
int min_cost(int *v, int *d,int total_node,int sid, struct area_info* azinfo)
{
	struct Netnode * matrix = azinfo->matrix;
	struct Netnode * node; 
	
	int i;
	int r_value =0; 	
	int temp = MAX_METRIC-1;
	int cur_ifindex=0,min_ifindex=MAX_INTERFACE;

	for(i=0;i<total_node;i++)
	{
/* when we met the equal node,we choose the min interface by Macro.Z, 2013-01-29 */
		if(v[i]==0 && d[i]<=temp && d[i]!=0)
		{
			node = matrix + sid * total_node + i;
			int rid = get_rid_by_index(sid, azinfo);
			int nrid = get_rid_by_index(node->nexthop[0], azinfo);
			struct link_info  * link = malloc_z(struct link_info);
			query_link_state_details(azinfo->azid, rid, nrid , link);
			cur_ifindex = link->ifid;
			if(d[i] == temp)
			{
				
				if(min_ifindex > cur_ifindex)
				{
					temp = d[i];
					min_ifindex = cur_ifindex;
					r_value = i;
				}
				
			}
			else
			{
				temp=d[i];
				min_ifindex = cur_ifindex;
				r_value =i;
			}
		}
	}
	return r_value;
}
int dijkstra(int s, struct area_info* az)
{
	int total     = az->router_count;
	int *distance = (int*)malloc(total*sizeof(int));
	int *visite   = (int*)malloc(total*sizeof(int));
	int i,j,w,sum;
	struct Netnode *node_w, *node_wj,*node_dst; 
	memset(distance,0,total*sizeof(int));
	memset(visite,0,  total*sizeof(int));
	
	struct Netnode * sourcenode = az->matrix + s* total;

	for(i=0;i<total;i++)
	{
		distance[i] = sourcenode[i].metric;
	}

	//init the nexthop array.
//	for(i=0;i<total;i++)
//	{
//		for(j=0;j<MAX_ECMP_LIMIT;j++)
//		 {
//			sourcenode[i].nexthop[j] = i;
//		 }
//	}
	// 
	//begin the dijkstra algorithm
	 for(i=0;i<total;i++)
	 {
	 	w = min_cost(visite,distance,total,s,az);
		node_w = az->matrix + s*total + w;
	 	visite[w] =1 ;
	 	for(j=0;j<total;j++)
	 	{
			if ( j == s )// 源地址和目的地址相同
			{
				distance[j] = 0;
				continue;
			}
			node_wj = az->matrix + w* total + j;
			node_dst = az->matrix + s* total + j;
	 		if(visite[j]==0)
			{
				 sum = distance[w]+node_wj->metric;
				 if(sum<=distance[j])
				 {
					if(sum<distance[j])
					{
						distance[j] = sum;
						nexthop_copy(node_w,node_dst);
					}
					else//存在等价多路径
					{
						equal_path_copy(node_w,node_dst);
					}
				 }
			}
	 	}
	 }
	 for(i=0;i<total;i++)
	 {
		 sourcenode[i].path_cost = distance[i];
		 /* choose one for ecmp */
		 if ( sourcenode[i].nexthop_count != 0 )
		 sourcenode[i].nextHop = sourcenode[i].nexthop[0];
		 else 
		 sourcenode[i].nextHop = s;
		 /**********************/
		 printf("D[%d] = %d",i,distance[i]);
	 }
	 printf("\n");
	 free(distance);
	 distance = NULL;
	 free(visite);
	 visite = NULL;
}

int equal_path_copy(struct Netnode *w,struct Netnode *dst)
{
	int i = 0 ;
	int index = i;
	int origin_ecmp_sum = dst->nexthop_count;
	for ( ; i < origin_ecmp_sum + w->nexthop_count ; i++, index++ )
	{
		//越早添加的等价等路径，越靠前
		if ( i < origin_ecmp_sum )
			continue;
		int j = 0;
		//后添加的要与前期的去重复
		int same = 0;
		for ( ; j< origin_ecmp_sum; j++ )
		{
			if ( w->nexthop[i-origin_ecmp_sum] == dst->nexthop[j] )
			{
				same = 1;
			}
		}
		if ( same )
		{
			index -= 1;
		}
		else 
		{
			dst->nexthop[index] = w->nexthop[i-origin_ecmp_sum];
			dst->nexthop_count++;
		}
	}
	return 0;
}

int nexthop_copy(struct Netnode *w,struct Netnode *dst)
{
	int i = 0;
	dst->nexthop_count= 0;
	for(i=0;i<w->nexthop_count;i++)
	{
		dst->nexthop[i] =	w->nexthop[i] ;
		dst->nexthop_count++;
		if(dst->nexthop_count> MAX_ECMP_LIMIT)
		{
			return 0;
		}
	}
	return 0;
}

struct Netpath* add_path_to_area_short_path(struct area_info *az)
{
	if ( az->shortpaths == NULL )
	{
		az->shortpaths = malloc_z(struct Netpath);
		return az->shortpaths;
	}
	struct Netpath * path = malloc_z(struct Netpath);
	path->next = az->shortpaths;
	az->shortpaths = path;
	return path;
}
int add_path_of_link( int link_index, struct Netpath* path, struct area_info * az)
{
	if ( az->ls_count < link_index ||  link_index < 0 )
		return -1;
	struct link_path_info* linkshadow = &az->linkshadows[link_index];
	if ( linkshadow->allshortpaths == NULL)
	{
		linkshadow->allshortpaths = malloc_z(struct paths_of_link);
		linkshadow->allshortpaths->shortpath = path;
	}
	else
	{
		struct paths_of_link *newpath = malloc_z(struct paths_of_link);
		newpath->shortpath = path;
		newpath->next = linkshadow->allshortpaths;
		linkshadow->allshortpaths = newpath;
	}
	linkshadow->shortpath_count++;
	return 0;
}
int record_short_path(struct area_info *az)
{
	int n = az->router_count;
	int i, j;
	struct Netnode * node;
	for(i=0; i< n ;i++)
		for (j = 0 ; j < n; j++)
		{
			node = az->matrix + i* n + j;
			if ( node->path_cost >= MAX_METRIC ||  node->metric == 0 )
				continue;
			struct Netpath * path = add_path_to_area_short_path(az);
			path->rid = get_rid_by_index(i , az);
			path->nrid = get_rid_by_index(j, az);
			path->nexthop_rid = node->nextHop;
			int ip_i = get_importance_value_of_router(path->rid, az);
			int ip_j = get_importance_value_of_router(path->nrid, az);
			path->importance_value = get_importance_value_of_router(path->rid, az) * 
				get_importance_value_of_router(path->nrid, az);
			path->distance_metric = node->path_cost;
			path->connection = CONNECTING;
			
			show_path(path);
			printf("path: ");

			int  nextHop = node->nextHop;
			int  thisnode = i;
			struct Netnode *nextnode; 
			struct Netnode *stepnode;
			while(thisnode != j && nextHop >= 0)
			{
				nextnode = az->matrix + thisnode*n + j;
				stepnode = az->matrix + thisnode*n + nextHop;
				int ls_index = stepnode->link_index;
				int r_i = get_rid_by_index( thisnode, az);
				int r_j = get_rid_by_index( nextHop , az);
				printf("%d->%d ", r_i , r_j);
				add_path_of_link(ls_index, path, az);
				thisnode = nextHop;
				nextHop = nextnode->nextHop;
			}
			printf("\n");
		}
}

int show_path(struct Netpath* path)
{
	printf("S:[%d] D:[%d] I:[%d] LEN:[%d] C:[%d]\n",
			path->rid, path->nrid, path->importance_value,
			path->distance_metric, path->connection);
}

int print_all_short_path(struct area_info* az)
{
	int n = az->router_count;
	int i , j ;
	for(i=0 ;i < n ;i++)
		for(j=0; j< n ; j++)
		{
			printf("%d %d:\necmp",get_rid_by_index(i,az), get_rid_by_index(j,az));
			int z = 0;
			struct Netnode * node = az->matrix + i* n + j;
			for( ; z <node->nexthop_count; z++)
				printf(" %d", get_rid_by_index(node->nexthop[z],az));
			printf("\n");
		}
}

int print_all_()
{
	/* code */
}
int path_compute_start()
{
	pthread_mutex_lock(&info_mutex);
	if ( already_computed )
		free_info_handle();

	init_lsdb_info();

	init_lidb_info();
	
	int i = 0;
	for ( ; i < info_handle->area_count; i++)
	{
		struct area_info *az = &info_handle->azinfo[i];
		int r = 0;
		for ( ; r < az->router_count ; r++)
		{
			printf("%d finished\n", r);
			dijkstra(r ,az);
		}
		//print_all_short_path(az);
		record_short_path(az);
	}
	printf("finished\n");
	already_computed = 1;
	spf_cpt_timer_cancel();
	pthread_mutex_unlock(&info_mutex);
}



int record_broken_path_down( struct link_info * link )
{
	int i = 0;
	struct area_info * az = get_area_by_areaid( link->areaid );
	if ( az == NULL ) return -1;
	for ( ; i < az->ls_count ; i++ )
	{
		struct link_info * ls = &az->lslist[i];
		if ( ls->rid == link->rid && ls->nrid == link->nrid 
				&& ls->ifid == link->ifid && ls->nifid == link->nifid )
		{// path the associate path down 
			struct paths_of_link* linkpath = az->linkshadows[i].allshortpaths;
			while(linkpath)
			{
				linkpath->shortpath->connection = UNCONNECTING;
				linkpath = linkpath->next;
			}
			break;
		}
	}
	return 0;
}


int get_importance_value_of_router(u32 rid, struct area_info * az)
{
	struct link_info * lslist = az->lslist;
	int ls_count  = az->ls_count;
	int iptvalue = 0;
	int i =0;
	for( ; i< ls_count ;i++)
	{
		if ( rid == lslist->rid && lslist->nifid  == 0 && lslist->nrid == 0)
			iptvalue+= lslist->metric;
		lslist++;
	}
	return iptvalue;
}


float get_net_destory_degree(struct area_info * az)
{
	int broken_value = 0;
	int net_value = 0;

	struct Netpath* path = az->shortpaths;
	while(path)
	{
		if ( path->connection == CONNECTING )
		{
			net_value += path->importance_value;
		}
		else 
		{
			net_value += path->importance_value;
			broken_value += path->importance_value;
		}
		path = path->next;
	}
	float svalue = (float)broken_value/(float)net_value;
	return svalue;
}
