#include <utils/utils.h>
#include <utils/common.h>
#include <db/dbutils.h>

#include <db/dbm_lsdb.h>


int main(int argc, const char *argv[])
{
	db_open();
	struct link_info * link = malloc_z(struct link_info);
	link->areaid = 0;
	link->rid = 1;
	link->nrid = 2;
	link->ifid = 1;
	link->nifid = 1;
	link->metric = 10;
	link->seq = 12345;
	link->status = 1;
	gettimeofday(&link->age,NULL);

	insert_link_state(link);
	int metric = query_link_state_metric(0,1,2);
	DEBUG(INFO,"TEST result: metric %d",metric);
	query_link_state_details(0,1,2,link);
	DEBUG(INFO,"TEST result: area[%d] routerid[%d] rif[%d] nrid[%d] nifid[%d] metric[%d] seq[%d] status[%d]",
			link->areaid, link->rid, link->ifid, link->nrid, link->nifid, link->metric, link->seq, link->status);
	int status = query_link_state_status(0,1,2);
	DEBUG(INFO,"TEST result: status %d",status);
	int routers_count = get_routers_count(0);
	DEBUG(INFO,"TEST result: routers_count %d",routers_count);
	struct lsdb_router * routers = get_routers(0);
	while(routers)
	{
		DEBUG(INFO,"TEST result: router id = %d", routers->rid);
		routers = routers->next;
	}
	int count ;
	struct link_info* links = get_link_states(0, &count);
	int i = 0;
	for(;i < count; i++)
	{
		DEBUG(INFO,"TEST result: rid[%d] ifid[%d] nrid[%d] nifid[%d] status[%d]\n",\
				links[i].rid, links[i].ifid, links[i].nrid, links[i].nifid, \
				links[i].status);
	}
	db_close();
}
