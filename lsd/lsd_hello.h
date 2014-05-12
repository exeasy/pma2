#ifndef __LSD_HELLO_H__
#     define __LSD_HELLO_H__

#include "lsd_if.h"
#include "lsd_socket.h"
#include <lib/thread.h>

//default hello value
#define H_HELLO_VAL 1
#define L_HELLO_VAL 20
#define H_DEAD_VAL 4
#define L_DEAD_VAL 80

#define TMP_VAR 100000

#define MAX_MESSAGE_LEN 2048

enum priority_type
{
	HIGH_PRIORITY_DETECTION,
	LOW_PRIORITY_DETECTION
};
enum lsd_status
{
	LSD_DISCONNETED = 0, LSD_CONNECTED = 1
};
/**local data structure**/
struct hello_high
{
	struct lsd_head lsd_head;
	u32 hello_interval;
	u32 dead_interval;
};
struct lsd_neighbor_info
{
	u32 rt_id;
	u32 if_id;
};

struct hello_low
{
	struct lsd_head lsd_head;
	u32 hello_interval;
	u32 dead_interval;
};
struct hello_master
{
	struct thread* dead_timer_high;
	struct thread* dead_timer_low;
	struct thread* hello_timer_high;
	struct thread* hello_timer_low;
	enum lsd_status high_status;
	enum lsd_status low_status;
	struct lsd_neighbor_info neighbor;
	u32 h_hello_val;
	u32 h_dead_val;
	u32 l_hello_val;
	u32 l_dead_val;
};


int hello_high_send(struct thread *thread);
int hello_low_send(struct thread *thread);
int dead_timer_high(struct thread *thread);
int dead_timer_low(struct thread *thread);



//void hello_on_eth_state_changed(struct backbone_eth* eth);

void hello_get_eth_state(struct backbone_eth* eth,
		enum eth_state* high,
		enum eth_state* low);
void hello_get_neighbor_info(struct backbone_eth* eth,
		struct lsd_neighbor_info* neighbor);

int lsd_is_connected(const struct lsdb_link_state* const lsa, id_t lsdb_handle);
//void lsd_add_hello_detection_handler(hello_detection_handler handler);

void hello_on_high_packet_received(struct backbone_eth* eth, struct lsd_head *head);
void hello_on_low_packet_received(struct backbone_eth* eth, struct lsd_head *head);
void hello_on_eth_state_changed(struct backbone_eth* eth);
//int update_timer_by_interface_id(int if_id);
//int update_timer_by_backbone(struct backbone_eth* eth);
//int update_timer_all_interface();


#endif /* __LSD_HELLO_H__ */
