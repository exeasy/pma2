#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#define MAX_MODULE_NUM 10
//模块注册表
struct module_register_table
{
	int m_id;		//模块ID
	int m_sockfd;		//socket fd
	char m_info[100];	//模块描述信息
};
void GetModuleInfo(char **msg, int *len);
int get_now_time(char**data, int *len); //caution to be free
#endif
