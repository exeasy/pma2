#include <utils/common.h>
#include <utils/utils.h>
#include "lsd_hello.h"
#include "lsd_cmn.h"
#include "lsd_if.h"
#include "lsd_event.h" 
#include "lsd_socket.h"

extern struct thread_master *master;

char sendbuf[MAX_MESSAGE_LEN];
declare_event_queue(detection_queue);
declare_event_queue(topology_queue);

//handle the event of hello state changing

void hello_on_eth_state_changed(struct backbone_eth* eth)
{
	if(!eth->hello_master)//insert init code.
		eth->hello_master = malloc_z(struct hello_master);
	struct hello_master* hello_master = eth->hello_master;
    printf("hello state changed\n");

	if(eth->_state == ETH_DOWN)
	{
		//THREAD_OFF(hello_master->dead_timer_high);
		//THREAD_OFF(hello_master->dead_timer_low);
		THREAD_OFF(hello_master->hello_timer_high);
		THREAD_OFF(hello_master->hello_timer_low);
	}
    else if(eth->_state == ETH_UP)
	{
		//if(check_global_hello_config()==0)
		//{
			hello_master->h_hello_val = H_HELLO_VAL;
			hello_master->l_hello_val = L_HELLO_VAL;
			hello_master->h_dead_val = H_DEAD_VAL;
			hello_master->l_dead_val = L_DEAD_VAL;
		//}
		//else
		//{
		//	int ifid = global_config.if_id;
		//	if(ifid == -1)
		//	{
		//		update_timer_by_backbone(eth);
		//	}
		//	else if(global_config.if_id == eth->interface_id)
		//	{
		//		update_timer_by_backbone(eth);
		//	}
//		}
		hello_master->hello_timer_high = thread_add_event(master, hello_high_send, eth, 0);
		hello_master->hello_timer_low = thread_add_event(master, hello_low_send, eth, 0);
	}
}

//send hello high message
int hello_high_send(struct thread *thread)
{

	struct backbone_eth* eth = (struct backbone_eth *)THREAD_ARG(thread);
	struct hello_high* hello_high = (struct hello_high*)sendbuf;
	struct hello_master* hello_master = eth->hello_master;

	hello_master->hello_timer_high = thread_add_timer_high_resolution(master, hello_high_send, eth, hello_master->h_hello_val*TMP_VAR);
	
    memset(sendbuf, 0, MAX_MESSAGE_LEN);

	hello_high->lsd_head.pktype = IC_MESSAGE_TYPE_HELLO_H;
	hello_high->lsd_head.pklen = sizeof(struct hello_high);

	hello_high->dead_interval = TMP_VAR * hello_master->h_dead_val;
	hello_high->hello_interval = TMP_VAR * hello_master->h_hello_val;

	lsd_send(eth, (struct lsd_head *)sendbuf);
	return 0;
}
//周期性发送hello low数据包
int hello_low_send(struct thread * thread)
{
	struct backbone_eth* eth = (struct backbone_eth *)THREAD_ARG(thread);
	struct hello_low* hello_low = (struct hello_low*)sendbuf;
	struct hello_master* hello_master = eth->hello_master;
	
	hello_master->hello_timer_low = thread_add_timer_high_resolution(master, hello_low_send, eth, TMP_VAR * hello_master->l_hello_val);

	memset(sendbuf, 0, MAX_MESSAGE_LEN);

	hello_low->lsd_head.pktype = IC_MESSAGE_TYPE_HELLO_L;
	hello_low->lsd_head.pklen = sizeof(struct hello_low);

	hello_low->dead_interval = TMP_VAR * hello_master->l_dead_val;
	hello_low->hello_interval = TMP_VAR * hello_master->l_hello_val;

	lsd_send(eth, (struct lsd_head *)sendbuf);
	return 0;
}

//receive and process hello high packet
void hello_on_high_packet_received(struct backbone_eth* eth, struct lsd_head *head)
{

	struct hello_high *hello_high = (struct hello_high *)head;
	struct hello_master* hello_master = eth->hello_master;

	if (head->pktype != IC_MESSAGE_TYPE_HELLO_H)
		return;

	hello_high = (struct hello_high *)head;

	THREAD_OFF(hello_master->dead_timer_high);
	hello_master->dead_timer_high = thread_add_timer_high_resolution(master, dead_timer_high, eth, TMP_VAR * hello_master->h_dead_val);

	if(hello_master->high_status == LSD_DISCONNETED)
	{
		hello_master->high_status = LSD_CONNECTED;
		hello_master->neighbor.rt_id = hello_high->lsd_head.r_id;
		hello_master->neighbor.if_id = hello_high->lsd_head.if_id;
		printf("Link Connected---Neighbor[%d]:[%d]\n",hello_master->neighbor.rt_id,hello_master->neighbor.if_id);
		char dst_addr[25],nexthop[25];
		//if(eth->neighbour->ctrl_addr.s_addr!=0){
		//inet_ntop(AF_INET,&eth->neighbour->ctrl_addr,dst_addr,25);
		//inet_ntop(AF_INET,&eth->neighbour->nexthop_addr,nexthop,25);
		//addStaticRoute(routerip, dst_addr,"255.255.255.255", nexthop);
		//}
		//eth->eth_state = ETH_UP;
		struct event_handler_item* item;
		for(item = detection_queue.next; item; item = item->next)
			((hello_detection_handler)(item->handler))(eth, HIGH_PRIORITY_DETECTION, LSD_CONNECTED);
	}
}

//receive and process hello low packet
void hello_on_low_packet_received(struct backbone_eth* eth, struct lsd_head *head)
{
	struct hello_low *hello_low = (struct hello_low *)head;
	struct hello_master* hello_master = eth->hello_master;

	if (head->pktype != IC_MESSAGE_TYPE_HELLO_L)
		return;

	hello_low = (struct hello_low *)head;

	THREAD_OFF(hello_master->dead_timer_low);
	hello_master->dead_timer_low = thread_add_timer_high_resolution(master, dead_timer_low, eth, TMP_VAR * hello_master->l_dead_val);

	if(hello_master->low_status == LSD_DISCONNETED)
	{
		hello_master->low_status = LSD_CONNECTED;
		hello_master->neighbor.rt_id = hello_low->lsd_head.r_id;
		hello_master->neighbor.if_id = hello_low->lsd_head.if_id;
		printf("CCCCCCCCCCCCCCCCCCCCConect\n");
		struct event_handler_item* item;
		for(item = detection_queue.next; item; item = item->next)
			((hello_detection_handler)(item->handler))(eth, LOW_PRIORITY_DETECTION, LSD_CONNECTED);
	}
}

//handle the dead high timeout event
int dead_timer_high(struct thread *thread)
{
	struct backbone_eth* eth = (struct backbone_eth*)THREAD_ARG(thread);
	struct hello_master* hello_master = eth->hello_master;
	hello_master->dead_timer_high = NULL;
	assert(hello_master->high_status == LSD_CONNECTED);
	hello_master->high_status = LSD_DISCONNETED;
	struct event_handler_item* item;
	printf("Link Disconnect~~~~~~~~~\n");
	//char dst_addr[25],nexthop[25];
	//inet_ntop(AF_INET,&eth->neighbor.ctrl_addr,dst_addr,25);
	//inet_ntop(AF_INET,&eth->neighbor->nexthop_addr,nexthop,25);
	//removeStaticRoute(routerip, dst_addr,"255.255.255.255", nexthop);
	//eth->eth_state = ETH_DOWN;
	for(item = detection_queue.next; item; item = item->next)
		((hello_detection_handler)(item->handler))(eth, HIGH_PRIORITY_DETECTION, LSD_DISCONNETED);
	return 0;
}

//handle the dead low timeout event
int dead_timer_low(struct thread *thread)
{
	struct backbone_eth* eth = (struct backbone_eth*)THREAD_ARG(thread);
	struct hello_master* hello_master = eth->hello_master;
	hello_master->dead_timer_low = NULL;
	assert(hello_master->low_status == LSD_CONNECTED);
	hello_master->low_status = LSD_DISCONNETED;
	struct event_handler_item* item;
	for(item = detection_queue.next; item; item = item->next)
		((hello_detection_handler)(item->handler))(eth, LOW_PRIORITY_DETECTION, LSD_DISCONNETED);
	return 0;
}

//get eth's hello state
void hello_get_eth_state(struct backbone_eth* eth,
		enum eth_state * high,
		enum eth_state * low)
{
	struct hello_master* hello_master = eth->hello_master;
	*high = hello_master->high_status;
	*low = hello_master->low_status;
}

////get eth's neighbor infomation
void hello_get_neighbor_info(struct backbone_eth* eth,
		struct lsd_neighbor_info* neighbor)
{
	struct hello_master* hello_master = eth->hello_master;
	*neighbor = hello_master->neighbor;
}


