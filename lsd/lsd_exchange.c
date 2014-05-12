#include <utils/utils.h>
#include <utils/common.h>
#include <lsdb/lsdb.h>
#include <lsd_event.h>
#include <lsd_utils.h>
#include <lsd_socket.h>
#include <lsd_hello.h>
#include <lsd_if.h>
#include <lsd_exchange.h>


declare_event_queue(exchange_queue);

int ack_time_excceed (struct thread *thread);
static int exchange_send (struct backbone_eth* eth);
static void exchange_send_ack(struct backbone_eth* eth,struct exchange *ex_header);
void link_lsa_received(struct exchange *ex_header, struct backbone_eth* bb_link);
void ex_ack_received(struct backbone_eth* eth,struct exchange *ex_header);
int exchange_send_start(struct backbone_eth* eth);
void exchange_terminate(struct backbone_eth* eth);
void exchange_pkt_received(struct backbone_eth* eth, struct lsd_head *head);

//handle the event of eth's state changing
void exchange_on_eth_state_changed(struct backbone_eth* eth)
{
	if(eth->exchange_master == NULL)
	{
		struct exchange_master *ex_master;
		assert((ex_master = malloc_z(struct exchange_master)) != NULL);
		ex_master->send_state = EXCHANGE_STATE_SEND_TERMINATED;
		ex_master->rcv_state = EXCHANGE_STATE_RCV_WAITING;

		ex_master->interface = eth;
		eth->exchange_master = ex_master;
	}
}
int exchange_send_start(struct backbone_eth* eth)
{
	int r;
	struct exchange_master *ex_master = eth->exchange_master;
	assert(ex_master != NULL);
	assert(ex_master->send_state == EXCHANGE_STATE_SEND_TERMINATED);

	ex_master->send_state = EXCHANGE_STATE_SEND_WAITING;
	id_t lsdb = lsdb_get_eth_handle(eth);
	r = cr_lsdb_get_links_state(&ex_master->links_state, lsdb);
	if(r != NO_ERR) return r;
	
	exchange_send(eth);
	return NO_ERR;
}

void exchange_terminate(struct backbone_eth* eth)
{
	struct exchange_master *ex_master = eth->exchange_master;
	assert(ex_master != NULL);

	FREE_LINK_LIST(ex_master->links_state);
	THREAD_OFF (ex_master->ack_ctrl);
	THREAD_OFF (ex_master->rcv_ctrl);
	ex_master->retry_times = 0;
	ex_master->pkg_no = 0;
	ex_master->send_state = EXCHANGE_STATE_SEND_TERMINATED;
	ex_master->rcv_state = EXCHANGE_STATE_RCV_TREMINATED;
	DEBUG(INFO,"exchange finished.");
}

void exchange_pkt_received(struct backbone_eth* eth, struct lsd_head *head)
{
	if(head->pktype != IC_MESSAGE_TYPE_EXCHANGE)
		return;

	struct exchange *ex_header = (struct exchange *)head;
	struct exchange_master *ex_master = eth->exchange_master;
	assert(ex_master != NULL);

	if (ex_header->type == EXCHANGE_PKG_TYPE_ACK)
	{
		if(ex_master && ex_master->send_state == EXCHANGE_STATE_SEND_WAITING)
			ex_ack_received(eth,ex_header);
		return;
	}
	//lsa received
	if(ex_master->rcv_state == EXCHANGE_STATE_RCV_TREMINATED)
		ex_master->rcv_state = EXCHANGE_STATE_RCV_WAITING;
	if(ex_master->rcv_state != EXCHANGE_STATE_RCV_WAITING)
		return;
	if(ex_header->type != EXCHANGE_PKG_TYPE_LINKS_STATE)
		return;

	link_lsa_received(ex_header, eth);
	exchange_send_ack(eth,ex_header);
	THREAD_OFF (ex_master->rcv_ctrl);

	if(!ex_header->more)
	{
		ex_master->rcv_state = EXCHANGE_STATE_RCV_END;
		if(ex_master->send_state == EXCHANGE_STATE_SEND_END)
		{
			struct event_handler_item* item;
			for(item = exchange_queue.next; item; item = item->next)
				((exchange_handler)(item->handler))(eth);
		}
	}
}


//send exchange message to the neighbors
static int exchange_send (struct backbone_eth* eth)
{
	struct exchange_master *ex_master = eth->exchange_master;

	//set lsd head
	struct lsd_head* lsd_head = (struct lsd_head*) ex_master->sendbuf;
	lsd_head->pktype = IC_MESSAGE_TYPE_EXCHANGE;
	lsd_head->pklen = sizeof(struct exchange);//now only the length of exchange_head
	//set exchange head
	struct exchange* ex_head = (struct exchange*)ex_master->sendbuf;
	ex_head->seq = ex_master->pkg_no;
	ex_head->length = 0;
	ex_head->type = EXCHANGE_PKG_TYPE_NO_DATA;

	//set lsa infomation
	struct link_state_adv* ex_pack = (struct link_state_adv *)((struct exchange *)ex_master->sendbuf + 1);
	while ((ex_head->length < LINK_LSA_CT_PER_PAK) && (ex_master->links_state))
	{
		lsdb_to_lsa_info(ex_master->links_state, ex_pack);
		ex_head->length++;
		lsd_head->pklen += sizeof(struct link_state_adv);
		ex_pack++;
		ex_head->type = EXCHANGE_PKG_TYPE_LINKS_STATE;

		struct cr_lsdb_link_state* tmp = ex_master->links_state;
		ex_master->links_state = ex_master->links_state->next;
		free(tmp);
	}
	ex_head->more = (ex_master->links_state ? TRUE : FALSE);
	ex_master->send_state = EXCHANGE_STATE_SEND_WAITING;
	ex_master->ack_ctrl = thread_add_timer_high_resolution(master, ack_time_excceed, ex_master, ACK_OVER_TIME);
	lsd_send (eth, lsd_head);
	return NO_ERR;
}


//timeout and resend exchange message
int ack_time_excceed (struct thread *thread)
{
	struct exchange_master *ex_master = (struct exchange_master*) THREAD_ARG (thread);
	if(!ex_master)
		return ILLEGAL_ARGS_ERR;
	ex_master->ack_ctrl = NULL;

	assert(ex_master->send_state == EXCHANGE_STATE_SEND_WAITING);

	ex_master->ack_ctrl = thread_add_timer_high_resolution(master, ack_time_excceed, ex_master, ACK_OVER_TIME);
	lsd_send(ex_master->interface, (struct lsd_head*)(ex_master->sendbuf));
	return NO_ERR;
}

//send ACK message
static void exchange_send_ack(struct backbone_eth* eth,struct exchange *ex_header)
{
	struct exchange ex_head;
	ex_head.lsd_hd.pklen = sizeof(struct exchange);
	ex_head.lsd_hd.pktype = IC_MESSAGE_TYPE_EXCHANGE;
	ex_head.seq = ex_header->seq;
	ex_head.length = 0;
	ex_head.more = 0;
	ex_head.type = EXCHANGE_PKG_TYPE_ACK;

	lsd_send(eth, (struct lsd_head*)&ex_head);
}

//receive the lsa
void link_lsa_received(struct exchange *ex_header, struct backbone_eth* bb_link)
{
	struct link_state_adv *ex_lsa;
	int i;
	//receive new lsa packet
	ex_lsa = (struct link_state_adv *)(((struct exchange*)ex_header) + 1);
	for (i = 0; i < ex_header->length; i++, ex_lsa++)
		accept_lsa_into_lsdb(bb_link, ex_lsa);
}

//receive and process the ack
void ex_ack_received(struct backbone_eth* eth,struct exchange *ex_header)
{
	struct exchange_master* ex_master = eth->exchange_master;

	if(ex_header->seq == ex_master->pkg_no)
	{
		THREAD_OFF (ex_master->ack_ctrl);
		ex_master->ack_ctrl = NULL;
		ex_master->retry_times = 0;
		if(ex_master->links_state)
		{
			ex_master->pkg_no++;
			exchange_send(eth);
		}else
		{
			ex_master->send_state = EXCHANGE_STATE_SEND_END;
			if(ex_master->rcv_state == EXCHANGE_STATE_RCV_END)
			{
				struct event_handler_item* item;
				for(item = exchange_queue.next; item; item = item->next)
					((exchange_handler)(item->handler))(eth);
			}
		}
	}
}
