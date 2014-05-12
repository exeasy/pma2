#include <utils/common.h>
#include <utils/utils.h>
#include <timer.h>
/*!
 * 定时器结构，包括定时器ID 和 该定时器的设置
 */
struct timer{
	u32 t_id;/*!<定时器ID*/
	struct pma_timer *timer;/*!<定时器的设置*/
};

/*!
 * 定时器的设置
 */
struct pma_timer{
	struct list_head list;	/*!<通用链表的头结构*/
	u32 timer_id;	/*!<定时器ID*/
	u32 time_val;	/*!<超时时间*/
	u32 timeout;	/*!<剩余的超时时间，当timeout为0时，调用回调函数，并重置为time_val*/
	timer_callback callback;	/*!<定时器的回调函数*/
	void *args;	/*!<回调函数的参数指针，可自定义参数的结构*/
	u32 len;	/*!<参数内容的长度*/
};


#define TIMER_MAX_COUNT 100 /*!<维护的最大定时器数目*/


static struct timer timer_arr[TIMER_MAX_COUNT];/*!<定时器数组*/
static u32 max_timer_id = 0;/*!<下次设置定时器时，定时器的ID*/
static u32 timer_num = 0;/*!<当前维护的定时器数目*/

static struct list_head timer_head = {&(timer_head), &(timer_head)};/*!<定义“定时器设置”连表的头节点*/


u32 set_timer(int time_val, timer_callback callback, void *args, u8 len)
{
	/*如果当前定时器的数目已经达到最大值，直接退出*/
	if (timer_num == TIMER_MAX_COUNT) {
		DEBUG(ERROR, "Maximum count Timer has been set.");
		return -1;
	}
	/*为新的定时器的设置 分配内存空间*/
	struct pma_timer *new_timer = (struct pma_timer *)malloc(sizeof(struct pma_timer));
	if (new_timer == NULL) {
		DEBUG(ERROR, "alloc memory for the new timer failed.");
		return -1;
	}

	/*设置新定时器的ID，下一次分配的定时ID增加*/
	new_timer->timer_id = max_timer_id++;
	/*设置新定时器的超时时间*/
	new_timer->time_val = time_val;
	new_timer->timeout = time_val;
	/*设置新定时器的回调函数*/
	new_timer->callback = callback;

	/*设置新定时器的回调函数的参数*/
	if (args != NULL) {
		/*为新定时器的参数分配内存*/
		new_timer->args = malloc(len);
		if (new_timer->args == NULL) {
			DEBUG(ERROR, "alloc memory for the new timer failed.");
			free(new_timer);
			return -1;
		}
		/*将参数复制到新分配的内存中*/
		memset(new_timer->args, 0, len);
		memcpy(new_timer->args, args, len);
		/*设置新定时器回调函数参数的长度*/
		new_timer->len = len;
	} else {
		new_timer->args = NULL;
		new_timer->len = 0;
	}


	/*将新的定时器设置添加到“设置连表中”*/
	list_add_tail_rcu(&timer_head, &(new_timer->list));

	/*将新的定时器添加到定时器数组中*/
	timer_arr[timer_num].t_id = new_timer->timer_id;
	timer_arr[timer_num].timer = new_timer;
	/*维护的定时器数目增加*/
	++timer_num;
	return new_timer->timer_id;
}

u32 update_timer(u32 t_id)
{
	/*如果当前维护的定时器数目为0，直接退出*/
	if (timer_num == 0) {
		DEBUG(ERROR, "There is no Timer in the program.");
		return -1;
	}

	/*重置指定定时器的超时时间*/
	int index;
	for (index = 0; index < timer_num; ++index) {
		if (timer_arr[index].t_id == t_id){
			/*超时时间 重新设置为最大超时时间*/
			timer_arr[index].timer->timeout = timer_arr[index].timer->time_val;
			return 0;
		}
	}	
	return 1; //没有该定时器
}
u32 modify_timer(u32 t_id, u32 timeout)
{
	/*如果当前维护的定时器数目为0，直接退出*/
	if (timer_num == 0) {
		DEBUG(ERROR, "There is no Timer in the program.");
		return -1;
	}
	/*修改指定定时器的超时时间*/
	int index;
	for (index = 0; index < timer_num; ++index) {
		if (timer_arr[index].t_id == t_id){
			/*最大超时时间间隔设置为新的超时时间*/
			timer_arr[index].timer->time_val = timeout;
			/*如果计时器正在计时的超时时间大于将要设置的超时时间，则将其设置为新的超时时间*/
			if (timer_arr[index].timer->timeout > timeout) {
				timer_arr[index].timer->timeout = timeout;
			}
			return 0;
		}
	}
	return 1; //没有该定时器
}
u32 unset_timer(u32 t_id)
{
	/*如果当前维护的定时器数目为0，直接退出*/
	if (timer_num == 0) {
		DEBUG(INFO, "There is no Timer in the program.");
		return 0;
	}
	/*修改指定定时器*/
	int index;
	for (index = 0; index < timer_num; ++index) {
		if (timer_arr[index].t_id == t_id){

			/*从定时器“设置链表”中删除指定定时器的设置*/
			list_del_rcu(&(timer_arr[index].timer->list));

			/*释放参数的内存*/
			free(timer_arr[index].timer->args);
			timer_arr[index].timer->args = NULL;

			/*释放定时器的内存*/
			free(timer_arr[index].timer);
			timer_arr[index].timer = NULL;

			/*从定时器数组中将定时器删除，并将其后面的定时器向前移动*/
			int j;
			int max = timer_num - 1;
			for (j = index; j < max; ++j) {
				timer_arr[j].t_id = timer_arr[j+1].t_id;
				timer_arr[j].timer = timer_arr[j+1].timer;
			}
			timer_arr[j].t_id = 0;
			timer_arr[j].timer = NULL;

			/*定时器数目减1*/
			--timer_num;
			break;
		}
	}	
	return 0;
}
u32 clear_timer(void)
{

	/*删除所有的定时器*/
	int index;
	for (index = 0; index < timer_num; ++index) {

		/*从定时器“设置链表”中删除指定定时器的设置*/
		list_del_rcu(&(timer_arr[index].timer->list));

		/*释放参数的内存*/
		free(timer_arr[index].timer->args);
		timer_arr[index].timer->args = NULL;

		/*释放定时器的内存*/
		free(timer_arr[index].timer);
		timer_arr[index].timer = NULL;
	}
	return 0;
}
/*!
 * 定时器的计时守护线程函数
 * \param args 输入参数的指针， 可自定义该参数结构
 * \return 因为是无限计时函数，永远不会返回
 */
static void *timer_thread_daemon(void * args);

/*!
 * 定时器初始化函数，运行计时守护线程
 * \return 无返回值
 */
int timer_init(void)  {
	pthread_t  timer_thread; 
	int i1 = 1;
	/*运行计时线程*/
	if (pthread_create(&timer_thread,NULL,timer_thread_daemon,&i1) != 0) {
		return -1;
	} else {
		return 0;
	}
}
/*!
 * 计时器缩减时间函数，缩减定时器数组中各个定时器的超时时间
 * \return 无返回值
 */
void exe_timer_callback(void) 
{
	int index;
	for (index = 0; index < timer_num; ++index) {
		/*缩减定时器的超时时间，如果超时时间到0，则重置为超时间隔，并调用该定时器的回调函数*/
		if ( --(timer_arr[index].timer->timeout) <= 0){
			timer_arr[index].timer->timeout = timer_arr[index].timer->time_val;
			pthread_t  cb_thread;
			/*另起一线程调用该定时器的回调函数*/
			pthread_create(&cb_thread,NULL,timer_arr[index].timer->callback,timer_arr[index].timer->args);
		}
	}
}
void *timer_thread_daemon(void *x){
	int err;
	/*时间结构，设置跳动时间为1s*/
	struct timespec t;
	t.tv_sec=1;
	t.tv_nsec=0;

	while(1)
	{
		/*事件检测函数，检测事件均为空，则永远检测不到事件的发生，则1s后超时，以此原理计时*/
		err = pselect(1,NULL,NULL,NULL,&t,NULL);
		if (err == -1) {
			DEBUG(INFO, "There is no Timer in the program.");
			return ((void *)-1);
		} else {
			/*处理计时器数组的超时时间*/
			exe_timer_callback();
		}
	}
	return NULL;
}

