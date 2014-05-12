#ifndef __LSD_SOCKET_H__
#     define __LSD_SOCKET_H__


#define MAX_MESSAGE_LEN 2048
/**IC packet type define**/
#define IC_MESSAGE_TYPE_HELLO_H 1
#define IC_MESSAGE_TYPE_HELLO_L 2
#define IC_MESSAGE_TYPE_EXCHANGE 3
#define IC_MESSAGE_TYPE_FLOOD 4
#define IC_MESSAGE_TYPE_LFA_CHANGE 5
#define IC_MESSAGE_TYPE_LFA_ADVERTISE 6

#define IC_H_PORT 19000
#define IC_L_PORT 18000

//declare_event_queue(pkt_rcv_queue)

/**head of packet**/
struct lsd_head
{
	u8 pktype;
	u8 version;
	u16 pklen;
	u32 r_id;
	u32 area_id;
	u32 if_id;
	u32 icsum;
};


/**functions**/

int create_server(int listen_port, int type);
int lsd_serv_sock();
int lsd_receive (struct thread *thread);
int lsd_send (struct backbone_eth* eth,struct lsd_head *head);


#endif /* __LSD_SOCKET_H__ */
