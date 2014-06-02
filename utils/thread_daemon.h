#ifndef THREAD_DAEMON_H

#define THREAD_DAEMON_H

typedef int (*daemon_ptr)(void* args, int type);

typedef struct PDaemon{
	pthread_t pid;
	daemon_ptr daemon; 
	void* args;
	int run_count;
	int min_interval;//the min time interval between two detections
	int max_interval;// the max  time interval between two detections
} PDaemon;

int create_daemon(PDaemon * p);

#endif /* end of include guard: THREAD_DAEMON_H */
