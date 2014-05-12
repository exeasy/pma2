#include <utils/common.h>
#include <utils/utils.h>
#include <ddc.h>
#include <policy/policy_table.h>
#include <db/dbm_lsdb.h>
#include <server/dbm.h>
#define DEFAULT_HELLO	10000000
#define DEFAULT_DEAD	40000000
#define MAX_INTERFACE_NUM 100

int is_router_need_protect = 0;
extern struct _policy_table *g_pt;
extern pthread_mutex_t info_mutex;
struct timeval dead_timer[MAX_INTERFACE_NUM];

int  protect_local_link(struct link_info* link, float destorydegree );
int reprotect_local_link(int areaid ,int routerid ,float destorydegree);

int fah_set_default_timer(u32 routerid, struct link_info *link)
{
	if ( !is_computed () )
	{
		printf("LIDB HAVEN'T INIT YET!!\n");
		return -1;
	}
	pthread_mutex_lock(&info_mutex);
	int areaid = link->areaid;
	int self_id = routerid;
	struct area_info *az= get_area_by_areaid(areaid);
	if( az == NULL )
	{
		pthread_mutex_unlock(&info_mutex);
		return -1;
	}
	int ifid = link->ifid;

	u32 hello_value = DEFAULT_HELLO;
	u32 dead_value = DEFAULT_DEAD;
	dead_timer[ifid].tv_sec = 0;
	dead_timer[ifid].tv_usec = 0;
	send_policy(ifid, hello_value, dead_value);
	pthread_mutex_unlock(&info_mutex);
}

int  fah_protect_link(u32 routerid , struct link_info* link)
{
	if ( !is_computed () )
	{
		printf("LIDB HAVEN'T INIT YET!!\n");
		return -1;
	}
	pthread_mutex_lock(&info_mutex);
	int areaid = link->areaid ;
	int self_id = routerid;
	struct area_info* az = get_area_by_areaid(areaid);
	if ( az == NULL )
	{
		pthread_mutex_unlock(&info_mutex);
		return -1;
	}


	//mark the path down
	record_broken_path_down(link);
	

	float destorydegree = get_net_destory_degree(az);

	// check the type of link
	if ( link->rid == self_id || link->nrid == self_id ) // direct connected backbone broken
		/* need protect */
	{
		is_router_need_protect = 1;
		//protect
		protect_local_link(link,destorydegree);
	}
	else if ( is_router_need_protect == 1 )// not directed connect 
	{
		reprotect_local_link(areaid, self_id , destorydegree );
	}
	else; //异地链路断，本地链路没断,不保护
	pthread_mutex_unlock(&info_mutex);
	return 0;
}

u32 recompute_dead_timer(int ifid , u32 dead_value)
{
	if ( ifid > MAX_INTERFACE_NUM ) return dead_value;
	struct timeval last_adjust_time = dead_timer[ifid];
	struct timeval timenow;
	u32 new_dead_value = dead_value ;
	if ( last_adjust_time.tv_sec == 0 && last_adjust_time.tv_usec == 0 )
	{//首次调整，记录时间并返回dead定时器
		if ( gettimeofday( &timenow , NULL ) == 0 )
			dead_timer[ifid] = timenow;
	}
	else 
	{//非首次调整，要减掉之前到当前的时间差
		if ( gettimeofday( &timenow, NULL ) == 0)
		{
			u32 usec_passed;
			usec_passed = 1000000 * ( timenow.tv_sec - last_adjust_time.tv_sec ) 
				+ timenow.tv_usec - last_adjust_time.tv_usec;
			u32  diff_time = dead_value - usec_passed ;
			if ( diff_time  > 0 )
			{
				new_dead_value = diff_time;
			}
			else 
				new_dead_value = 0;
		}
	}
	return new_dead_value;
}

int  protect_local_link(struct link_info* link, float destorydegree )
{
	struct area_info * az = get_area_by_areaid(link->areaid);
	//get policy
    int key = (int)(destorydegree*100.0);
	int ret = find_policy_table( g_pt, key);
	if ( ret == -1 )
		return -1;

	int ifid = link->ifid; //要保护的接口id

	u32 hello_value; 
    u32 dead_value; 
    if ( ret == 0) 
    {
	    hello_value = DEFAULT_HELLO;
	    dead_value = DEFAULT_DEAD;
    }
    else 
    {
	    hello_value = g_pt->p_element[ret].value[0];
	    dead_value = g_pt->p_element[ret].value[1];
    }
	dead_value = recompute_dead_timer(ifid,dead_value);
	DEBUG(INFO, "Destory Degree is [%f], choosed policy 'Hello:[%d] Dead:[%d]'\n",
			destorydegree, hello_value, dead_value );
	
	
	//send policy
	send_policy( ifid , hello_value , dead_value );

}

int reprotect_local_link(int areaid ,int routerid ,float destorydegree)
{
	struct area_info* az = get_area_by_areaid(areaid);
	if ( az == NULL ) return -1;
	int ls_count = az->ls_count;
	int i = 0;
	for ( ;i < ls_count ; i++ )
	{
		struct link_info* info = &az->lslist[i];
		if (info->status != 3  &&  info->rid == routerid && info->nrid != 0 && info->nifid != 0 )
		{
			protect_local_link(info, destorydegree );
		}
	}
}
