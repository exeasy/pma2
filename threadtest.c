#include <utils/utils.h>
#include <utils/common.h>
#include <pthread.h>
#include <lib/thread.h>

struct thread_master * master;
int preformace_test()
{
	thread_add_timer(master, preformace_test, NULL, 5);
	printf("This is a preformance test\n");
	sleep(3);
	printf("finish write\n");
}
int read_test()
{
	printf("receive a packet\n");
}
void func_p()
{
	struct thread thread;
	pthread_t pid;
	while(thread_fetch(master, &thread))
	{
		thread_call(&thread);
	}
		//pthread_create(&pid, NULL, thread_call, (void*)&thread);
}
int main()
{
	master = thread_master_create();

	thread_add_read(master,read_test, NULL ,0);

	thread_add_timer(master,preformace_test,NULL,5);
//	pthread_t pid;
//	pthread_create(&pid, NULL, func_p, NULL);
	func_p();
	sleep(1000);
	return 0;
}
