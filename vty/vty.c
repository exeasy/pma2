#include <utils/common.h>
#include <utils/utils.h>
#include <conf/conf.h>
#include <comm/interior_daemon.h>
#include <mmanager/module_manager.h>
#include <vty.h>

int vty_server();
extern struct conf pma_conf;

int vty_serv_sock;
CMsg cmd_msg_list[20] = 
{
	{CMD_SET,SET_HELP_STR,-1,NULL},
	{CMD_COMM_TYPE,SET_BM_COMM,0,set_bm_commtype},
	{CMD_PMSIP,SET_BM_PMSIP,0,set_bm_pmserverip},
	{CMD_PMSPORT,SET_BM_PORT,0,set_bm_pmserverport},
	{CMD_ALCAIP,SET_BM_ALCA_IP,0,set_bm_alcaaddress},
	{CMD_ALCAPORT,SET_BM_ALCA_PORT,0,set_bm_alcaport},
	{CMD_LOGSRVIP,SET_LOGSRV_IP,0,set_bm_logsrvaddress},
	{CMD_LOGSRVPORT,SET_LOGSRV_PORT,0,set_bm_logsrvport},
	{CMD_ROUTERIP,SET_ROUTERIP,0,set_routerip},
	{CMD_LOCALIP,SET_LOCALIP,0,set_localip},
	{CMD_LOCALMASK,SET_LOCALMASK,0,set_localmask},
	{CMD_OUTSIDE,SET_OUTSIDE,0,set_outside},
	{CMD_HELLO_H,SET_IC_HELLO_H,0,set_ic_hello_h_timeval},
	{CMD_HELLO_L,SET_IC_HELLO_L,0,set_ic_hello_l_timeval},
	{CMD_DEAD_H,SET_IC_DEAD_H,0,set_ic_dead_h_timeval},
	{CMD_DEAD_L,SET_IC_DEAD_L,0,set_ic_dead_l_timeval},
	{CMD_SNAP,SET_DBM_SNAP,0,set_dbm_snapshoot_timeval},
	{CMD_POLICY,SET_DBM_POLICY,0,set_dbm_policytype},
	{CMD_OSPF_HELLO,SET_PEA_HELLO,0,set_pea_ospf_hello},
	{CMD_OSPF_DEAD,SET_PEA_DEAD,0,set_pea_ospf_dead},
	{NULL,NULL,-1,NULL}
};
int vty_init()
{
	int i=0;
	for(;cmd_msg_list[i].cmd!=NULL;i++)
	{
		printf("Command:%s %s\n",cmd_msg_list[i].cmd,cmd_msg_list[i].help_info);
	}
	pthread_t pid;
	pthread_create(&pid,NULL,(void*)vty_server_start,NULL);
	return 0;
}

int set_value_by_type(int sockfd,int ops,void* value)
{
	if(ops<0||ops>20)return -1;
	if(NULL != cmd_msg_list[ops].cmd)
	{
		value_setter setter = cmd_msg_list[ops].func_ptr;
		int ret = setter(value);
		if(ret == 0)
			sendMessage(sockfd,"Set Successfully!\n");
		else
			sendMessage(sockfd,"The value not right!\n");
	}
}

int handle_cmd_word(int sockfd,char* word,char* val)
{
	int i=0;
	printf("%s\n",word);
	for(;cmd_msg_list[i].cmd!=NULL;i++)
	{
		printf("%s\n",cmd_msg_list[i].cmd);
		if(strncmp(cmd_msg_list[i].cmd,word,strlen(cmd_msg_list[i].cmd))==0)
		{
			if(val == NULL||*val == '?')
			{
				printf("%s",cmd_msg_list[i].help_info);
				sendMessage(sockfd,cmd_msg_list[i].help_info);
				return -1;//lack param
			}
			else
			{
				set_value_by_type(sockfd,i,val);
				return 0;
			}
		}
	}
	sendMessage(sockfd,cmd_msg_list[0].help_info);
	return -2;//not found command
}

void vty_con_handle(void* args)
{
	//	signal(SIGPIPE,SIG_IGN);
	int client_fd = *(int*)args;
	fd_set con_fd;
	int err = -1;
	int exit_flag = 0;
	while(1)
	{
		if(exit_flag)break;
		int maxfd = 20;
		FD_ZERO(&con_fd);
		FD_SET(client_fd,&con_fd);
		err = select(maxfd,&con_fd,NULL,NULL,0);
		switch(err)
		{
			case 0:break;
			case -1:
				   {
					   printf("Client close the socket\n");
					   close(client_fd);
					   if(errno == EPIPE)
						   return NULL;
					   break;
				   }
			default:
				   {
					   char* cmd = (char*)malloc(50);
					   memset(cmd,0,50);
					   int len = 50;
					   readMessage(client_fd,&cmd,&len);
					   printf("%s\n",cmd);
					   if(cmd == NULL)
					   {
						   break;
					   }
					   else
					   {
						   char* com = strtok(cmd," ");
						   if(com == NULL)break;
						   else
						   {
							   if(strncmp(com,"set",3)==0)
							   {
								   printf("get the token set\n");
								   char* word = strtok(NULL," ");//command
								   if(word==NULL)
								   {
									   sendMessage(client_fd,SET_HELP_STR);
								   }
								   else
								   {
									   char* val = strtok(NULL," ");//value
									   int ret = handle_cmd_word(client_fd,word,val);

								   }
							   }
							   else if(strncmp(com,"show",4)==0)
							   {
								   //sendMessage(client_fd,"The page not ready for use\n");
								   show_total_module(&client_fd);
								   show_all(client_fd);
							   }
							   else if(strncmp(com,"exit",4)==0)
							   {
								   sendMessage(client_fd,"Logging out.....\n");
								   close(client_fd);
								   return;
							   }
						   }
					   }
					   free(cmd);
					   break;
				   }
		}
	}
	close(client_fd);
	pthread_detach(pthread_self());

}
int make_socket_non_blocking (int sfd)
{
	int flags, s;

	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror ("fcntl");
		return -1;
	}

	return 0;
}


int vty_server_start()
{
	int ret=0;
	struct sockaddr_in vty_serv_addr;
	ret = socket(AF_INET,SOCK_STREAM,0);
	if(ret < 0)
	{
		DEBUG(ERROR,"Cant's create socket");
		return -1;
	}
	bzero(&vty_serv_addr,sizeof(struct sockaddr_in));
	vty_serv_addr.sin_family = AF_INET;
	vty_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	vty_serv_addr.sin_port = htons(VTY_PORT);
	int serv_len = sizeof(struct sockaddr_in);
	int n=1; 
	setsockopt(ret, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int)); 
	vty_serv_sock = ret;
	ret = bind (vty_serv_sock, (struct sockaddr*)&vty_serv_addr, serv_len);
	if (ret != 0)
	{
		DEBUG(ERROR,"Bind error");
		return -1;
	}
	ret  = listen (vty_serv_sock, SOMAXCONN);
	if(ret <0)
	{
		DEBUG(ERROR,"Listen Error");
		return -1;
	}
	fd_set scan_fd;
	int err = -1;
	while(1)
	{
		int maxfd = 20;
		FD_ZERO(&scan_fd);
		FD_SET(vty_serv_sock,&scan_fd);
		pthread_t pt_recv;
		err = select(maxfd,&scan_fd,NULL,NULL,0);
		switch(err)
		{
			case 0:break;
			case -1:break;
			default:
					{
						struct sockaddr in_addr;
						socklen_t in_len;
						int infd;
						in_len = sizeof in_addr;
						infd = accept(vty_serv_sock,&in_addr,&in_len);
						if(infd == -1)
						{
							DEBUG(INFO,"Receive a wrong connection from vty");				
						}
						else
						{
							make_socket_non_blocking(infd);
							DEBUG(INFO,"Receive a new connection from vty");
							sendMessage(infd,"hello,this is pma3.0 cofig vty\n");
							pthread_create(&pt_recv,NULL,(void*)vty_con_handle,(void*)&infd);
						}
						break;
					}
		}
	}

}	


int sendMessage(int sockfd,char* msg)
{
	int len = strlen(msg);
	send(sockfd,msg,len,0);
}

int readMessage(int sockfd,char** msg,int* len){
	int tmp_len = *len;
	char * buf = *msg;
	while(1)
	{
		tmp_len = recv(sockfd,buf,tmp_len,0);
		if(tmp_len <0)
		{
			*len = 0;
			*msg = NULL;
			return 0;
		}
		else if(tmp_len == *len){
			buf = (char*)realloc(*msg+tmp_len,tmp_len);
			*len += tmp_len;
		}
		else
		{
			break;
		}
	}
}


int set_bm_commtype(void* args)
{
	char* data = (char*)args;
	int type = atoi(data);
	if(type <0 || type >2)return -1;
	pma_conf.comm_type = type;
	return 0;
}
int set_bm_pmserverip(void* args)
{
	char* addr = (char*)args;
	int len = strlen(addr)-1;
	if(len == 0|| addr == NULL)return -1;
	strncpy(pma_conf.server_ip,addr,len);
	return 0;
}
int set_bm_pmserverport(void* args)
{
	char* data = (char*)args;
	int port = atoi(data);
	pma_conf.server_port = port;
	return 0;
}
int set_bm_alcaaddress(void* args)
{
	char* addr = (char*)args;
	int len = strlen(addr)-1;//not the \n
	if(len == 0|| addr == NULL)return -1;
	strncpy(pma_conf.alca_ip,addr,len);
	return 0;
}
int set_bm_alcaport(void* args)
{
	char* data = (char*)args;
	int port = atoi(data);
	pma_conf.alca_port= port;
	return 0;
}
int set_bm_logsrvaddress(void* args)
{
	char* addr = (char*)args;
	int len = strlen(addr)-1;//not the \n
	if(len == 0|| addr == NULL)return -1;
	strncpy(pma_conf.logsrv_ip,addr,len);
	return 0;
}
int set_bm_logsrvport(void* args)
{
	char* data = (char*)args;
	int port = atoi(data);
	pma_conf.logsrv_port = port;
	return 0;
}
int set_routerip(void* args)
{
	char* addr = (char*)args;
	int len = strlen(addr)-1;//not the \n
	if(len == 0|| addr == NULL)return -1;
	inet_pton(AF_INET,addr, &pma_conf.ic_config.router_ip);
	inet_pton(AF_INET,addr, &pma_conf.pea_config.router_ip);
	return 0;
}
int set_localip(void *args)
{
	char* addr = (char*)args;
	int len = strlen(addr)-1;//not the \n
	if(len == 0|| addr == NULL)return -1;
	inet_pton(AF_INET,addr, &pma_conf.ic_config.local_ip);
	return 0;
}
int set_localmask(void *args)
{
	char* addr = (char*)args;
	int len = strlen(addr)-1;//not the \n
	if(len == 0|| addr == NULL)return -1;
	inet_pton(AF_INET,addr, &pma_conf.ic_config.netmask);
	return 0;
}
int set_outside(void *args)
{
	char * sval = (char*)args;
	int val = atoi(sval);
	if ( val != 0 && val != 1 )return -1;
	pma_conf.ic_config.outside = val;
	return 0;
}
int set_ic_hello_h_timeval(void* args)
{
	char* data = (char*)args;
	int val = atoi(data);
	if(val<=0||val>=100)return -1;
	pma_conf.ic_config.h_hello_val = val;
	send_config_to_module(ICM);
	return 0;
}
int set_ic_hello_l_timeval(void* args)
{
	char* data = (char*)args;
	int val = atoi(data);
	if(val<=0||val>=400)return -1;
	pma_conf.ic_config.l_hello_val = val;
	send_config_to_module(ICM);
	return 0;
}

int set_ic_dead_h_timeval(void* args)
{
	char* data = (char*)args;
	int val = atoi(data);
	if(val<=0||val>=400)return -1;
	pma_conf.ic_config.h_dead_val = val;
	send_config_to_module(ICM);
	return 0;
}
int set_ic_dead_l_timeval(void* args)
{
	char* data = (char*)args;
	int val = atoi(data);
	if(val<=0||val>=1600)return -1;
	pma_conf.ic_config.l_dead_val = val;
	send_config_to_module(ICM);
	return 0;
}

int set_dbm_snapshoot_timeval(void* args)
{
	char* data = (char*)args;
	int val = atoi(data);
	if(val <0||val>60)return -1;
	pma_conf.dbm_config.snapshoot_timeval = val;
	send_config_to_module(DBM);
	return 0;
}
int set_dbm_policytype(void* args)
{
	char* data = (char*)args;
	int val = atoi(data);
	if(val<=0||val>3)return -1;
	pma_conf.dbm_config.policy_type = val;
	send_config_to_module(DBM);
	return 1;
}
int set_pea_ospf_hello(void* args)
{
	char* data = (char*)args;
	int val = atoi(data);
	if(val<=0||val>255)return -1;
	//pma_conf.dbm_config.policy_type = val;
	return 1;
}
int set_pea_ospf_dead(void* args)
{
	char* data = (char*)args;
	int val = atoi(data);
	if(val<=0||val>255)return -1;
	//pma_conf.dbm_config.policy_type = val;
	return 1;
}
int show_total_module(void* args)
{
	char* msg;
	int len;
	int fd = *(int*)args;
	if(fd<=0)return -1;
	GetModuleInfo(&msg,&len);
	if(msg==NULL||len==0)
		sendMessage(fd,"No Module has been registered!\n");
	else sendMessage(fd,msg);
	return 0;
}

int show_bm_config(void* args)
{
	int fd = *(int*)args;
	if(fd<=0)return -1;

}


int show_all(int fd)
{
	char* data = (char*)malloc(1024);
	memset(data,0,1024);
	int len = 0;
	sprintf(data,"PMA3.0 Configuration:\n");
	len = strlen(data);
	sprintf(data+len,"Commucation method: %s\n",(pma_conf.comm_type==0)?"TCP":((pma_conf.comm_type==1)?"ALCA":"BOTH"));
	len = strlen(data);
	sprintf(data+len,"PMS ip address: %s\n",pma_conf.server_ip);
	len = strlen(data);
	sprintf(data+len,"PMS port: %d\n",pma_conf.server_port);
	len = strlen(data);
	sprintf(data+len,"ALCA ip address: %s\n",pma_conf.alca_ip);
	len = strlen(data);
	sprintf(data+len,"ALCA port: %d\n",pma_conf.alca_port);
	len = strlen(data);
	sprintf(data+len,"IC Module's hello(High) send time: 4 (100ms)\n");
	len = strlen(data);
	sprintf(data+len,"IC Module's hello(Low)  send time: 10 (100ms)\n");
	len = strlen(data);
	sprintf(data+len,"IC Module's dead (High) send time: 10 (100ms)\n");
	len = strlen(data);
	sprintf(data+len,"IC Module's dead (Low) send time: 80 (100ms)\n");
	len = strlen(data);
	sprintf(data+len,"DBM's snapshoot send time: %d (s)\n",pma_conf.dbm_config.snapshoot_timeval);
	len = strlen(data);
	sprintf(data+len,"DBM's policy type: %s\n",(pma_conf.dbm_config.policy_type==1)?"Destory degree":"Complex DDC");
	sendMessage(fd,data);
	free(data);
}


int show_ic_config(void* args){}

int show_ic_interface(void* args){}

int show_ic_routertable(void* args){}

int show_ic_neighbor(void* args){}

int show_dbm_config(void* args){}
int show_dbm_policy_dm(void* args){}
int show_dbm_lsdb(void* args){}
int show_dbm_lidb(void* args){}
int show_spf_history(void* args){}

int show_pea_history(void* args){}

