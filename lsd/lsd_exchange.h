#ifndef __LSD_EXCHANGE_H__
#     define __LSD_EXCHANGE_H__

#define MAX_EXCHANGE_BUF_LEN MAX_MESSAGE_LEN   //lsd_socket.h
#define ACK_OVER_TIME 20000
#define RCV_OVER_TIME 1000000
#define MAX_RETRY_TIMES 10
#define ACCESS_LSA_CT_PER_PAK 1
#define LINK_LSA_CT_PER_PAK 1

enum exchange_send_state{
	EXCHANGE_STATE_SEND_TERMINATED = 0,
	EXCHANGE_STATE_SEND_WAITING,
	EXCHANGE_STATE_SEND_END
};
enum exchange_rcv_state{
	EXCHANGE_STATE_RCV_TREMINATED = 0,
	EXCHANGE_STATE_RCV_WAITING,
	EXCHANGE_STATE_RCV_END
};
struct exchange_master
{
	//for send
	enum exchange_send_state send_state;
	enum exchange_rcv_state rcv_state;
	struct cr_lsdb_link_state* links_state;

	u32 pkg_no;
	u8 has_more;
	char sendbuf[MAX_EXCHANGE_BUF_LEN];
	struct thread* ack_ctrl;
	struct thread* rcv_ctrl;
	int retry_times;
	//mapping
	struct backbone_eth* interface;
};

#define EXCHANGE_PKG_TYPE_LINKS_STATE 0
#define EXCHANGE_PKG_TYPE_NO_DATA 2
#define EXCHANGE_PKG_TYPE_ACK 3

struct exchange
{
	struct lsd_head lsd_hd;
	u8 type;
	u8 more;
	u16 length;
	u32 seq;

	
};
extern struct thread_master *master;

int ack_time_excceed (struct thread *thread);
int rcv_time_excceed (struct thread *thread);
void finish_rcv(enum exchange_rcv_state state, struct exchange_master *ex_master);
void finish_send(enum exchange_send_state state, struct exchange_master *ex_master);

int exchange_send_start(struct backbone_eth* eth);
void exchange_terminate(struct backbone_eth* eth);

void exchange_pkt_received(struct backbone_eth* eth, struct lsd_head *head);

void exchange_on_eth_state_changed(struct backbone_eth* eth);
#endif /* __LSD_EXCHANGE_H__ */
