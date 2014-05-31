#include <utils/utils.h>
#include <utils/common.h>
#include <utils/list.h>
#include <lsdb/lsdb.h>
#include <lsd_socket.h>
#include <lsd_utils.h>
#include <lsd_if.h>
#include <lsd_hello.h>
#include "lsd_flood.h"


#define LSA_CT_PER_PAK 5
#define FLOOD_BUFFER_TIME 100000

extern struct lsd_router g_lsd_router;
extern struct thread_master *master;
struct thread* periodctrl = NULL;

void flood_on_eth_state_changed(struct backbone_eth* eth)
{
	if(eth->flood_master == NULL)
	{
		struct flood_master* master;
		assert((master = malloc_z(struct flood_master)) != NULL);
		master->state = FLOOD_TERMINATED;
		master->interface = eth;
		cr_init_list_head(&master->queue);
		master->interface = eth;
		eth->flood_master = master;
	}
	static int called = FALSE;
	if(!called)
	{
		flood_periodically_flood_start();
		called = TRUE;
	}
}

//send ack message
static void flood_send_ack(struct backbone_eth* link, struct flood* header)
{
	struct flood new;
	new.lsd_hd.pklen = sizeof(struct flood);
	new.lsd_hd.pktype = IC_MESSAGE_TYPE_FLOOD;
	new.seq = header->seq;
	new.size = 0;
	new.type = FLOOD_PKG_TYPE_ACK;

	lsd_send(link, (struct lsd_head*)&new);
}

//timeout and resend ack
int flood_ack_timeout(struct thread *thread)
{
	struct flood_master *flood_master = (struct flood_master*) THREAD_ARG (thread);
	lsd_send(flood_master->interface, (struct lsd_head*)flood_master->sendbuf);
	flood_master->ack_ctrl = thread_add_timer_high_resolution(master, flood_ack_timeout, flood_master, ACK_OVER_TIME);
	return 0;
}
//when ack received, just send another lsa
int flood_send_lsa(struct thread *thread)
{
	struct flood_master *flood_master = (struct flood_master*) THREAD_ARG (thread);
	flood_master->snd_ctrl = NULL;
	memset(flood_master->sendbuf, 0, MAX_FLOOD_BUF_LEN);
	struct lsd_head* lsd_head;
	struct flood* flood_head;
	//set ic head
	lsd_head = (struct lsd_head*)flood_master->sendbuf;
	lsd_head->pktype = IC_MESSAGE_TYPE_FLOOD;
	lsd_head->pklen = sizeof(struct flood);
	flood_head = (struct flood*)flood_master->sendbuf;
	flood_head->seq = ++flood_master->pkg_no;
	flood_head->type = FLOOD_PKG_TYPE_LSA;

	struct link_state_adv* ptr = (struct link_state_adv*)((struct flood *)flood_master->sendbuf + 1);
	int count = 0;
	while(!cr_list_empty(&flood_master->queue) && (count < LSA_CT_PER_PAK))
	{
		count++;
		struct flood_snd_item* entry = CR_LIST_ENTRY(flood_master->queue.next, struct flood_snd_item, ptrs);
		cr_list_del(flood_master->queue.next);
		*ptr = entry->lsa;
		free(entry);
		ptr++;
		lsd_head->pklen += sizeof(struct link_state_adv);
	}
	flood_head->size = count;
	DEBUG(INFO,"[TX]SEQ %d, COUNT %d.", flood_head->seq, count);

	flood_master->state = FLOOD_ACK_WAITING;
	flood_master->ack_ctrl = thread_add_timer_high_resolution(master, flood_ack_timeout, flood_master, ACK_OVER_TIME);
	lsd_send (flood_master->interface, lsd_head);
	return 0;
}

//receive and process the flood lsa
void flood_pkt_received(struct backbone_eth* eth, struct lsd_head *head)
{
	if (head->pktype != IC_MESSAGE_TYPE_FLOOD)
		return;
	struct flood* flood_head = (struct flood*)head;
	struct flood_master* flood_master = eth->flood_master;

	if(flood_head->type == FLOOD_PKG_TYPE_ACK && flood_master->state == FLOOD_ACK_WAITING)
	{
		DEBUG(INFO,"[RX]SEQ %d ACK %d.", flood_head->seq, flood_master->pkg_no);
		if(flood_head->seq == flood_master->pkg_no)
		{
			THREAD_OFF(flood_master->ack_ctrl);
			if(cr_list_empty(&flood_master->queue))
			{
				flood_master->state = FLOOD_READY;
			}else{
				flood_master->state = FLOOD_BUSY;
				flood_master->snd_ctrl =
					thread_add_timer_high_resolution(master, flood_send_lsa, flood_master, 0);
			}
		}
	}else if(flood_head->type == FLOOD_PKG_TYPE_LSA)
	{
		flood_send_ack(eth, flood_head);
		int count = flood_head->size;
		struct link_state_adv* ptr = (struct link_state_adv*)(flood_head + 1);
		DEBUG(INFO,"[RX]SEQ %d, COUNT %d.", flood_head->seq, count);
		while(count-- > 0)
		{
			/*******************see the lsa state*********************/
			if ( (ptr->state==1) && (ptr->key.n_rt_id!=0) )
			{

				struct link_state_key link_key;
				link_key.rt_id = ptr->key.rt_id;
				link_key.if_id = ptr->key.if_id;
				link_key.n_rt_id = ptr->key.n_rt_id;
				link_key.n_if_id = ptr->key.n_if_id;

				struct link_state_key link_key_reverse;
				link_key_reverse.rt_id = ptr->key.n_rt_id;
				link_key_reverse.if_id = ptr->key.n_if_id;
				link_key_reverse.n_rt_id = ptr->key.rt_id;
				link_key_reverse.n_if_id = ptr->key.if_id;

				id_t lsdb_handle  = lsdb_get_eth_handle(eth);
//				id_t lidb_handle  = eth->az->lidb;
				struct cr_lsdb_link_state result;
				struct cr_lsdb_link_state result_reverse;

				int ret = cr_lsdb_link_state_find(&link_key, &result, lsdb_handle);
				int ret_rev = cr_lsdb_link_state_find(&link_key_reverse, &result_reverse, lsdb_handle);

				if((ret==NOT_FOUND_ERR)||((result.state!=0)&&(result.state!=1)))
				{
					if((ret_rev==NOT_FOUND_ERR)||((result_reverse.state!=0)&&(result_reverse.state!=1)))
					{
						struct cr_lsdb_link_state link;
						link.key = link_key;

						//get destroy degree
						int destroy_degree;
						//ddc_pathStateDown(&ic_link, lidb_handle, lsdb_handle);
						//destroy_degree = ddc_get_net_destroy_degree(lidb_handle);
						if(destroy_degree > 0)
						{
							//printf("ret:%d, ret_rev:%d, result:%d, result_reverse:%d\n", ret, ret_rev, result.state, result_reverse.state);

							printf("\n\n[FAH]Destroy degree became:%d!!!\n\nBegin to reprotect the link %d:%d-->%d:%d, state:%d!!!\n\n", destroy_degree, ptr->key.rt_id, ptr->key.if_id, ptr->key.n_rt_id, ptr->key.n_if_id, ptr->state);
							//fah_reprotect(lsdb_handle);
						}
					}

				}
			}
			/*********************************************************/
			accept_lsa_into_lsdb(eth, ptr++);
		}

	}
}

//add lsa into the queue
void flood_add_flood_lsa(struct link_state_adv* lsa, struct backbone_eth* eth)
{
	struct flood_master* flood_master = eth->flood_master;
	if(!flood_master) return;
	DEBUG(INFO,"<Flood ADD %d ?>\t\t%d:%d--%d:%d seq(%d)\t", eth->_ifid, lsa->key.rt_id, lsa->key.if_id,
			lsa->key.n_rt_id, lsa->key.n_if_id, lsa->seq);
	if(flood_master->state == FLOOD_TERMINATED)
	{
		DEBUG(INFO,"\t<QUEUE ADD FAILIED.ETH %d FLOOD TERMINATE>", eth->_ifid);
		return;
	}

	DEBUG(INFO,"\t<QUEUE ADD OK.>");
	struct flood_snd_item* item = malloc_z(struct flood_snd_item);
	item->lsa = *lsa;
	cr_list_add_tail(&item->ptrs, &flood_master->queue);

	if(flood_master->state == FLOOD_READY)
	{
		flood_master->state = FLOOD_BUSY;
		flood_master->snd_ctrl =
			thread_add_timer_high_resolution(master, flood_send_lsa, flood_master, FLOOD_BUFFER_TIME);
	}
}

//terminate the flood process
void flood_terminate(struct backbone_eth* eth)
{
	struct flood_master* flood_master = eth->flood_master;
	THREAD_OFF(flood_master->ack_ctrl);
	THREAD_OFF(flood_master->snd_ctrl);
	flood_master->state = FLOOD_TERMINATED;
	while(!cr_list_empty(&flood_master->queue))
	{
		struct flood_snd_item* entry = CR_LIST_ENTRY(flood_master->queue.next, struct flood_snd_item, ptrs);
		cr_list_del(flood_master->queue.next);
		free(entry);
	}
}

//receive and spread the lsa of heighbors
void flood_lsa_spread(struct link_state_adv* lsa, struct autonomous_zone* oz)
{
	struct backbone_eth* eths = oz->backbones;
	while(eths)
	{
		flood_add_flood_lsa(lsa, eths);
		eths = eths->next;
	}
}

//forward the lsa of neighbors
void flood_lsa_forward(struct link_state_adv* lsa, struct backbone_eth* eth)
{
	struct autonomous_zone* az = eth->az;
	struct backbone_eth* eths = az->backbones;

	while(eths)
	{
		if(eths != eth)
			flood_add_flood_lsa(lsa, eths);
		eths = eths->next;
	}
}

//receive and spread the lsa of heighbors
void flood_spread_access_link_state(struct access_eth* eth, u8 state)
{
	struct link_state_key key;
	key.if_id = eth->_ifid;
	key.n_rt_id = 0;
	key.n_if_id = 0;
	struct autonomous_zone* azs = g_lsd_router.azs;
	while(azs)
	{
		key.rt_id = azs->device_id;
		id_t lsdb = azs->lsdb;

		struct cr_lsdb_link_state link_state;
		if(cr_lsdb_link_state_find(&key, &link_state, lsdb) == NO_ERR)
		{
			link_state.state = state;
			link_state.seq++;
			cr_lsdb_link_state_update(&link_state, azs->lsdb);
		}else{
			link_state.key = key;
			get_address_of_backbone(eth, &link_state.addr);
			link_state.metric = get_metric_of_backbone(eth);
			link_state.seq = 1;
			link_state.state = state;
			cr_lsdb_link_state_add(&link_state, lsdb);
		}

		struct link_state_adv ic_lsa;
		lsdb_to_lsa_info(&link_state, &ic_lsa);
		flood_lsa_spread(&ic_lsa, azs);

		azs = azs->next;
	}
}

//receive and spread the lsa of heighbors
void flood_spread_backbone_link_state(struct backbone_eth* eth, u8 state)
{
	struct link_state_key key;
	struct lsd_neighbor_info neighbor;
	key.if_id = eth->_ifid;
	key.rt_id = eth->az->device_id;
	hello_get_neighbor_info(eth, &neighbor);
	assert(neighbor.rt_id != 0);
	key.n_rt_id = neighbor.rt_id;
	key.n_if_id = neighbor.if_id;

	id_t lsdb = lsdb_get_eth_handle(eth);
	struct cr_lsdb_link_state link_state;
	if(cr_lsdb_link_state_find(&key, &link_state, lsdb) == NO_ERR)
	{
		link_state.state = state;
		link_state.seq++;
		cr_lsdb_link_state_update(&link_state, lsdb);
	}else{
		link_state.key = key;
		get_address_of_backbone(eth, &link_state.addr);
		link_state.metric = get_metric_of_backbone(eth);
		link_state.seq = 1;
		link_state.state = state;
		cr_lsdb_link_state_add(&link_state, lsdb);
	}

	struct link_state_adv ic_lsa;
	lsdb_to_lsa_info(&link_state, &ic_lsa);
	flood_lsa_spread(&ic_lsa, eth->az);
}

//spread local links' state
int spread_local_link_state(struct thread* t)
{
	DEBUG(INFO,"PERIODICALLY FLOOD CALLED...");
	struct access_eth* acs = g_lsd_router.accesses;
	while(acs)
	{
		struct link_state_key key;
		key.if_id = acs->_ifid;
		key.n_rt_id = 0;
		key.n_if_id = 0;
		struct autonomous_zone* azs = g_lsd_router.azs;
		while(azs)
		{
			key.rt_id = azs->device_id;
			id_t lsdb = azs->lsdb;

			struct cr_lsdb_link_state link_state;
			if(cr_lsdb_link_state_find(&key, &link_state, lsdb) == NO_ERR)
			{
				struct link_state_adv ic_lsa;
				link_state.seq++;
				lsdb_to_lsa_info(&link_state, &ic_lsa);
				flood_lsa_spread(&ic_lsa, azs);
				cr_lsdb_link_state_update(&link_state, lsdb);
			}
			azs = azs->next;
		}
		acs = acs->next;
	}

	struct autonomous_zone* azs = g_lsd_router.azs;
	while(azs)
	{
		struct backbone_eth* bbs = azs->backbones;
		id_t lsdb = azs->lsdb;

		while(bbs)
		{
			struct link_state_key key;
			key.rt_id = azs->device_id;
			key.if_id = bbs->_ifid;
			struct lsd_neighbor_info neighbor;
			hello_get_neighbor_info(bbs, &neighbor);
			if(neighbor.rt_id != 0)
			{
				key.n_rt_id = neighbor.rt_id;
				key.n_if_id = neighbor.if_id;
				struct cr_lsdb_link_state link_state;
				if(cr_lsdb_link_state_find(&key, &link_state, lsdb) == NO_ERR)
				{
					struct link_state_adv ic_lsa;
					link_state.seq++;
					lsdb_to_lsa_info(&link_state, &ic_lsa);
					flood_lsa_spread(&ic_lsa, azs);
					cr_lsdb_link_state_update(&link_state, lsdb);
				}
			}
			bbs = bbs->next;
		}
		azs = azs->next;
	}

	periodctrl = thread_add_timer_high_resolution(master, spread_local_link_state, NULL, 30000000);
	return 0;
}

//start the flood process
void flood_start(struct backbone_eth* eth)
{
	flood_terminate(eth);
	struct flood_master* flood_master = eth->flood_master;
	assert(flood_master->state == FLOOD_TERMINATED);
	flood_master->state = FLOOD_READY;
}

//start periodically flood
void flood_periodically_flood_start()
{
	flood_periodically_flood_terminate();
	periodctrl = thread_add_timer_high_resolution(master, spread_local_link_state, NULL, 30000000);
	DEBUG(INFO,"PERIODICALLY FLOOD START...");
}

//terminate periodically flood
void flood_periodically_flood_terminate()
{
	THREAD_OFF(periodctrl);
}
