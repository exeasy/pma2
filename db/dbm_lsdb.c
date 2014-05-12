#include <utils/common.h>
#include <utils/utils.h>
#include <sqlite3.h>
#include <dbutils.h>
#include "dbm_lsdb.h"

extern sqlite3 *db;
extern struct query_reuslt * query_result ;

int init_empty_lsdb()
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return -1;
	}
	char cmd[MAX_QUERY_LEN];
	sprintf(cmd, "delete from pma_lsdb;");
	printf("Query: %s\n", cmd);
	struct query_result * result = malloc_z(struct query_result);
	db_query( cmd );
	return 0;
//	show_result(result);
}

int insert_link_state( struct link_info * link)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return -1;
	}
	char cmd[MAX_QUERY_LEN];
	char * timestr = PRINTTIME(link->age);
	sprintf(cmd, "insert into pma_lsdb values(\"%d\", \"%d\", \"%d\", \"%d\",\" %d\",\" %d\", \"%d\", \"%d\", \"%s\");",\
	link->areaid, link->rid, link->nrid, link->ifid, link->nifid,\
	link->metric, link->status, link->seq, timestr);
	free(timestr);
//	printf("Query: %s\n", cmd);
//	struct query_result * result = malloc_z(struct query_result);
	db_query( cmd );
	return 0;
}
int update_link_state( struct link_info * link)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return -1;
	}
	char cmd[MAX_QUERY_LEN];
	char * timestr = PRINTTIME(link->age);
	sprintf(cmd, "update pma_lsdb\
			set metric = \"%d\",\
			status = \"%d\", \
			seq = \"%d\", \
			age = \"%s\" \
			where areaid = \"%d\" and rid = \"%d\" and nrid = \"%d\"  and ifid = \"%d\"and nifid = \"%d\";",\
	link->metric, link->status, link->seq, timestr,
	link->areaid, link->rid, link->nrid, link->ifid, link->nifid);
	free(timestr);
//	printf("Query: %s\n", cmd);
//	struct query_result * result = malloc_z(struct query_result);
	db_query( cmd );
	return 0;
}

int* get_areaes(int *count)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return NULL;
	}
	if ( count == NULL)
		return NULL;
	*count = 0;
	char cmd[MAX_QUERY_LEN];
	sprintf(cmd, "select distinct areaid from pma_lsdb;");
	printf("Query: %s\n", cmd);
	struct query_result * result = malloc_z(struct query_result);
	db_query_call( cmd , result);
	if( result->total == 0)
		return NULL;
	else
	{
		*count = result->total;
		int *arealist = (int*)malloc(*count*sizeof(int));
		struct sqlresult * s = result->result;
		int i = 0;
		for( ; i < *count ;i++)
		{
			arealist[i] = atoi( s->data[0] );
			s = s->next;
		}
		return arealist;
	}
	return NULL;
	
}
int get_routers_count(int areaid)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return -1;
	}
	char cmd[MAX_QUERY_LEN];
	sprintf(cmd, "select count( distinct rid) from pma_lsdb where areaid = %d;",\
	areaid);
	printf("Query: %s\n", cmd);
	struct query_result * result = malloc_z(struct query_result);
	db_query_call( cmd , result);
	if( result->total == 0)
		return -1;
	else if ( result->total == 1)
	{
		int total = atoi(result->result->data[0]);
		return total;
	}
	return -1;
	
}
struct lsdb_router* get_routers(int areaid)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return NULL;
	}
	char cmd[MAX_QUERY_LEN];
	sprintf(cmd, "select distinct rid from pma_lsdb where areaid = %d order by rid asc;",\
	areaid);
	printf("Query: %s\n", cmd);
	struct query_result * result = malloc_z(struct query_result);
	db_query_call( cmd , result);
	if( result->total == 0)
		return NULL;
	else
	{
		struct lsdb_router * routers = malloc_z(struct lsdb_router);
		struct lsdb_router * h = routers;
		int num = result->total;
		struct sqlresult * s = result->result;
		int i = 0;
		for (i = 0; i < num; i++) {
			/* code */
			h->rid = atoi(s->data[0]);
			if(i!=num-1)
				h->next = malloc_z(struct lsdb_router);
			else h->next = NULL;
			h = h->next;
			s = s->next;
		}
		return routers;
	}
	return NULL;
}

int query_link_state_status(u32 areaid, u32 routerid ,u32 neighbor_routerid)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return -1;
	}
	char cmd[MAX_QUERY_LEN];
	sprintf(cmd, "select status from pma_lsdb where areaid = %d and rid = %d and nrid = %d;",\
	areaid, routerid, neighbor_routerid);
	printf("Query: %s\n", cmd);
	struct query_result * result = malloc_z(struct query_result);
	db_query_call( cmd , result);
	if( result->total == 0)
		return -1;
	else if ( result->total == 1)
	{
		int status = atoi(result->result->data[0]);
		return status;
	}
	return -1;
	//show_result(result);
}

int query_link_state_metric(u32 areaid, u32 routerid ,u32 neighbor_routerid)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return -1;
	}
	char cmd[MAX_QUERY_LEN];
	sprintf(cmd, "select metric from pma_lsdb where	areaid = %d and rid = %d and nrid = %d;",\
			areaid, routerid, neighbor_routerid);
	printf("Query: %s\n", cmd);
	struct query_result * result = malloc_z(struct query_result);
	db_query_call( cmd , result);
	if( result->total == 0)
		return -1;
	else if ( result->total == 1)
	{
		int metric = atoi(result->result->data[0]);
		return metric;
	}
	return -1;
//	show_result(result);
}

int query_link_state_details(u32 areaid, u32 routerid ,u32 neighbor_routerid, struct link_info* link)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return -1;
	}
	char cmd[MAX_QUERY_LEN];
	sprintf(cmd, "select * from pma_lsdb where	areaid = %d and rid = %d and nrid = %d;",\
			areaid, routerid, neighbor_routerid);
	printf("Query: %s\n", cmd);
	struct query_result * result = malloc_z(struct query_result);
	db_query_call( cmd , result);
	if( result->total == 0)
		return -1;
	else if ( result->total == 1)
	{
		link->areaid = atoi(result->result->data[0]);
		link->rid = atoi(result->result->data[1]);
		link->nrid = atoi(result->result->data[2]);
		link->ifid = atoi(result->result->data[3]);
		link->nifid = atoi(result->result->data[4]);
		link->metric = atoi(result->result->data[5]);
		link->status = atoi(result->result->data[6]);
		link->seq = atoi(result->result->data[7]);
		return 0;
	}
	show_result(result);
	return -1;
}
struct link_info* get_link_states(u32 areaid, int *count)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return NULL;
	}
	if ( count == NULL )
	{
		return NULL;
	}
	*count = 0;
	char cmd[MAX_QUERY_LEN];
	sprintf(cmd, "select * from pma_lsdb where	areaid = %d;",\
			areaid);
	printf("Query: %s\n", cmd);
	struct query_result * result = malloc_z(struct query_result);
	db_query_call( cmd , result);
	if( result->total == 0)
		return NULL;
	else
	{
		*count = result->total;
		struct link_info* links = (struct link_info*)malloc(*count*sizeof(struct link_info));
		int i = 0;
		struct sqlresult* s = result->result;
		for( ; i< *count ;i++)
		{
		links[i].areaid = atoi(s->data[0]);
		links[i].rid = atoi(s->data[1]);
		links[i].nrid = atoi(s->data[2]);
		links[i].ifid = atoi(s->data[3]);
		links[i].nifid = atoi(s->data[4]);
		links[i].metric = atoi(s->data[5]);
		links[i].status = atoi(s->data[6]);
		links[i].seq = atoi(s->data[7]);
		s = s->next;
		}
		return links;
	}
	show_result(result);
	return -1;
}
struct link_info* get_link_states_of_router(u32 areaid,u32 routerid , int *count)
{
	if ( db == NULL )
	{
		printf("DB not inited\n");
		return NULL;
	}
	if ( count == NULL )
	{
		return NULL;
	}
	*count = 0;
	char cmd[MAX_QUERY_LEN];
	sprintf(cmd, "select * from pma_lsdb where	areaid = %d and rid = %d;",\
			areaid, routerid);
	printf("Query: %s\n", cmd);
	struct query_result * result = malloc_z(struct query_result);
	db_query_call( cmd , result);
	if( result->total == 0)
		return NULL;
	else
	{
		*count = result->total;
		struct link_info* links = (struct link_info*)malloc(*count*sizeof(struct link_info));
		int i = 0;
		struct sqlresult* s = result->result;
		for( ; i< *count ;i++)
		{
		links[i].areaid = atoi(s->data[0]);
		links[i].rid = atoi(s->data[1]);
		links[i].nrid = atoi(s->data[2]);
		links[i].ifid = atoi(s->data[3]);
		links[i].nifid = atoi(s->data[4]);
		links[i].metric = atoi(s->data[5]);
		links[i].status = atoi(s->data[6]);
		links[i].seq = atoi(s->data[7]);
		s = s->next;
		}
		return links;
	}
	show_result(result);
	return -1;
}

void pack_lsdb_xml_header(char *buff,int *len, u32 routerid )
{
	sprintf(buff+*len,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
						"<SNAPSHOT>\n"
						"<ROUTER id=\"%d\">\n",routerid);
	*len = strlen(buff);
	time_t raw_time;
	struct tm* time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);
	sprintf(buff+*len,"<timestamp>%4d-%d-%d %d:%d:%d</timestamp>\n",
			1900+time_info->tm_year,
			1+time_info->tm_mon,
			time_info->tm_mday,
			time_info->tm_hour,
			time_info->tm_min,
			time_info->tm_sec);
	*len = strlen(buff);
    sprintf(buff+*len, "<LSDB>\n");
    *len = strlen(buff);
}
void pack_lsdb_xml_router_of_area(char *buff, int *len , u32 routerid, u32 areaid)
{
    int num;
	struct link_info* links = get_link_states_of_router(areaid, routerid, &num);
	int i ;
	for(i = 0; i< num ;i ++)
	{
		sprintf(buff+*len, "\t\t<interface id=\"%d\">\n",links->ifid);
		*len = strlen(buff);
		sprintf(buff+*len, "\t\t\t<area_id>%d</area_id>\n",links->areaid);
		*len = strlen(buff);
		sprintf(buff+*len, "\t\t\t<neighbor_id>%d</neighbor_id>\n",links->nrid);
		*len = strlen(buff);
		sprintf(buff+*len, "\t\t\t<neighbor_if>%d</neighbor_if>\n",links->nifid);
		*len = strlen(buff);
		sprintf(buff+*len, "\t\t\t<metric>%d</metric>\n",links->metric);
		*len = strlen(buff);
		sprintf(buff+*len, "\t\t\t<state>%d</state>\n",links->status);
		*len = strlen(buff);
		sprintf(buff+*len, "\t\t</interface>\n");
		*len = strlen(buff);
		links++;
	}
}

void pack_lsdb_xml_footer(char *buff , int *len)
{
    sprintf(buff+*len, "</LSDB>\n");
    *len = strlen(buff);
	sprintf(buff+*len, "</ROUTER>\n</SNAPSHOT>\n");
	*len = strlen(buff);
}
/*  type == 1 local router links state */
/*  type == 0 local area likns state */
char* format_lsdb_to_xml(u32 areaid, u32 routerid,int type)
{
	int count;
	struct link_info* links = get_link_states(areaid, &count);
	struct lsdb_router* routers = get_routers(areaid);

	char* buff = (char*)malloc(count*200+1024);
	memset(buff, 0x00 , count*200+1024);
	int len =0;
	pack_lsdb_xml_header(buff, &len, routerid);
	while(routers)
	{
		int num = 0;
		int rid = routers->rid;
		if ( type == 1 && rid != routerid )
		{
			routers = routers->next;
			continue;
		}
		sprintf(buff+len, "\t<router id=\"%d\">\n",rid);
		len = strlen(buff);
        pack_lsdb_xml_router_of_area(buff, &len, rid, areaid);
		sprintf(buff+len, "\t</router>\n");
		len = strlen(buff);
		routers  = routers->next;
	}
    pack_lsdb_xml_footer(buff, &len);
	return buff;
}

char* format_all_area_lsdb_to_xml(int routerid)
{
    int count = 0;
    int * azs = get_areaes(&count);
    int i = 0;
    char *buff = (char*)malloc(count*200*15+1024);
    memset(buff, 0x00 , count*200*15+1024);
    int len = 0;
    pack_lsdb_xml_header(buff, &len, routerid);
    sprintf(buff+len, "\t<router id=\"%d\">\n",routerid);
    len = strlen(buff);
    for( i ; i < count ;i ++)
    {
        pack_lsdb_xml_router_of_area(buff, &len, routerid, azs[i]);
    }
    sprintf(buff+len ,"\t</router>\n");
        len = strlen(buff);
        pack_lsdb_xml_footer(buff,&len);
        return buff;
}
