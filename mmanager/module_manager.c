#include <utils/utils.h>
#include <utils/common.h>
#include <comm/interior_daemon.h>
#include <module_manager.h>

struct module_register_table g_m_register_table[100];
char *modlist[] = {"", "ICM", "DBM", "PEM", "BM", "PMS" };

int module_manager_init()
{
	memset(g_m_register_table, 0x00, sizeof(g_m_register_table));
}


/* add a new registered module into register table
 * */
	int
insert_register_table( int m_id, int m_sockfd, char *info )
{

	if( m_id <=0 || m_id >= MAX_MODULE_NUM )
	{
		DEBUG(ERROR, "module id is wrong=[%d]", m_id );
		return -1;
	}

	if( m_sockfd <= 0 )
	{
		DEBUG(ERROR , "module socket fd is wrong=[%d]", m_sockfd );
		return -1;
	}

	DEBUG( INFO, "module[%s] registered",  modlist[m_id] );
	int exist = 0;// if the module reg more than one time ,then simply update the infomation
	if(g_m_register_table[m_id].m_id == m_id)exist = 1;
	g_m_register_table[m_id].m_id = m_id;
	g_m_register_table[m_id].m_sockfd = m_sockfd;
	char* m_info = (char*)g_m_register_table[m_id].m_info;
	int len = 0;
	get_now_time(&m_info,&len);
	DEBUG(INFO,"Registered:%s\n",m_info);
	return exist;
}
/* find module infomation of given module_id
 *  return struct module_register_table*
 *  */
	struct module_register_table *
find_register_table_by_id( int m_id )
{
	if( m_id <= 0 || m_id >= MAX_MODULE_NUM )
	{
		DEBUG(ERROR, "module id is wrong=[%d]\n", m_id );
		return NULL;
	}

	struct module_register_table *p = NULL;

	if( g_m_register_table[m_id].m_id != 0 )
	{
		p = &g_m_register_table[m_id];
	}

	return p;
}

void GetModuleInfo(char** msg,int* len)
{
	int i = 0;
	*msg = (char*)malloc(1024);
	memset(*msg,0,1024);
	char* p = *msg;
	for(; i<MAX_MODULE_NUM; i++)
	{
		if(g_m_register_table[i].m_id == i&&g_m_register_table[i].m_sockfd >2)
		{
			switch(i)
			{
				case ICM:
					sprintf(p,"ICM(Infomation Module) is Registered\nSocket FD:[%d]\nRegister Time:%s\n",g_m_register_table[i].m_sockfd,g_m_register_table[i].m_info);
					p += strlen(p);
					break;
				case DBM:
					sprintf(p,"DBM(DataBase Module) is Registered\nSocket FD:[%d]\nRegister Time:%s\n",g_m_register_table[i].m_sockfd,g_m_register_table[i].m_info);
					p += strlen(p);
					break;
				case PEM:
					sprintf(p,"PEM(Policy Excuate Module) is Registered\nSocket FD:[%d]\nRegister Time:%s\n",g_m_register_table[i].m_sockfd,g_m_register_table[i].m_info);
					p += strlen(p);
					break;
				default:
					break;
			}
		}
		*len = strlen(*msg);
	}
}

int get_now_time(char**data, int *len) //caution to be free
{
	time_t raw_time;
	struct tm* time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);
	sprintf(*data,"%4d-%d-%d %d:%d:%d",
			1900+time_info->tm_year,
			1+time_info->tm_mon,
			time_info->tm_mday,
			time_info->tm_hour,
			time_info->tm_min,
			time_info->tm_sec);
	*len = strlen(*data);
	return 1;
}
