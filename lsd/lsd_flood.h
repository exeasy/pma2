#ifndef __LSD_FLOOD_H__
#     define __LSD_FLOOD_H__

#define MAX_FLOOD_BUF_LEN MAX_MESSAGE_LEN

#define FLOOD_PKG_TYPE_LSA 0
#define FLOOD_PKG_TYPE_ACK 1

#define ACK_OVER_TIME 20*1000000
struct flood
{
	struct lsd_head lsd_hd;
	u8 type;
	u32 seq;
	u32 size;
};

enum flood_state{
	FLOOD_READY, FLOOD_BUSY, FLOOD_ACK_WAITING, FLOOD_TERMINATED
};
struct flood_snd_item
{
	struct link_state_adv lsa;
	struct cr_list_head ptrs;
};
struct flood_master
{
	struct cr_list_head queue;
	char sendbuf[MAX_FLOOD_BUF_LEN];

	u32 pkg_no;
	enum flood_state state;

	struct thread* ack_ctrl;
	struct thread* snd_ctrl;
	//mapping
	struct backbone_eth* interface;
};

void flood_lsa_forward(struct link_state_adv* lsa, struct backbone_eth* eth);
void flood_add_flood_lsa(struct link_state_adv* lsa, struct backbone_eth* eth);
    
void flood_on_eth_state_changed(struct backbone_eth* eth);
void flood_pkt_received(struct backbone_eth* eth, struct lsd_head *head);

#endif /* __LSD_FLOOD_H__ */
