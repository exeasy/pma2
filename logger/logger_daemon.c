#include <utils/common.h>
#include "logger.h"
#include <conf/conf.h>
#include <comm/header.h>
#include <comm/comm.h>
#include <comm/comm_utils.h>
#include <utils/utils.h>
#include <stdio.h>
#include <sys/param.h>
#include <lib/shmem.h>
#include <lib/semph.h>
#include <control/control.h>

int fork_daemon(void)
{
	int pid;
	int i;
	if((pid = fork()))
		return 0;
		//exit(0);
	else if (pid < 0)
		exit(1);
	setsid();
//	if((pid = fork()))
//		exit(0);
//	else if (pid < 0)
//		exit(1);
//	for(i=0 ; i < NOFILE ; i++)
//		close(i);

//	chdir("/tmp");
//	umask(0);
	return 1;
}


int log_daemon()
{
	//create the share memory
	// int shmid = create_shm("/tmp/pma.id", MAX_BLOCK * BLOCK_SIZE );
	//create the semphore 
	// int ret = init_sems("/tmp/pma.id", 3 );
	//here we create three semphore , one for lock, the other two for block resources control
	// sem[0] lock
	// ret = set_sem_value(0, 1);
	// 
	// sem[1] varys{0,MAX_BLOCK}
	// ret = set_sem_value(1, MAX_BLOCK);
	// 
	// sem[2] varys{0,MAX_BLOCK}
	// ret = 
	//loop to check if there is data to read
	//while (1)
	//{
	//	p_sem(EMPTY);
	//	log_reader();
	//	v_sem(FULL);
	//}
	//int ret = fork_daemon();
	//if(ret == 0)
	//{
	//	return 0;
	//}
	while(1)
	{
		//Here I use v(FULL) 
		//cause I dont find a way to set 
		//a max value of sephore
		//Init the s(FULL) to MAX then reverse 
		//the opt(p,v) can achieve the purpose,too.
		p_sem(FULL);
		struct packet pkt;
		memset(&pkt, 0 , sizeof(pkt));
		char* msg = log_reader();
		char* addr = get_logsrv_address();
		strcpy(pkt.ip,addr);
		int port = get_logsrv_port();
		pkt.port = port;
		pkt.len = strlen(msg);
		pkt.timeout = 10;
		pkt.ops_type = PKT_TYPE_EVENT_NOTIFY;
		printf("PKT type:%d, Len: %d\n",pkt.ops_type, pkt.len);
		printf("LogSrv %s:%d\n",addr,port);
		int sid = create_connect(addr, port);
		if(sid > 0)
		{
			pkt.sockfd = sid;
			int ret = encapsulate_packet(&pkt, msg);
			if(ret == 0){
				ret = send_packet(&pkt);
				if(ret == -1)
				{
					printf("Send Packet Error\n");
				}
				close_connect(sid);
			}
			else{
				printf("encapsulate failed\n");
			}
		}

		free(msg);
		v_sem(EMPTY);
	}
}
