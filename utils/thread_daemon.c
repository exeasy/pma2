#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "thread_daemon.h"


int delay_time(int n)//ms
{
	struct timeval delay;
	delay.tv_sec = n/1000;
	delay.tv_usec = n%1000 * 1000; // n ms
	select(0, NULL, NULL, NULL, &delay);
}

void daemon_loop(PDaemon *p)
{
	int max_loop_count = p->max_interval/p->min_interval;
	while(1){
		p->daemon(p->args, 1);
		p->run_count = (p->run_count+1)%max_loop_count;	
		if(p->run_count == max_loop_count - 1)
		{
			p->daemon(p->args, 2);
		}
		delay_time(p->min_interval);
	}
}

int create_daemon(PDaemon * p)
{
	pthread_create(&(p->pid), NULL, 
			daemon_loop, p); 
}


