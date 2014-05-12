#ifndef __LSD_EVENT_H__
#     define __LSD_EVENT_H__
#include "lsd_cmn.h"
#include <lib/event.h>
typedef void (*access_eth_state_handler)(struct access_eth* eth);
typedef void (*backbone_eth_state_handler)(struct backbone_eth* eth);
typedef void (*pkt_rcv_handler)(struct backbone_eth* link,struct lsd_head* head);
typedef void (*hello_detection_handler)(struct backbone_eth* eth,enum priority_type type, enum lsd_status status);
typedef void (*exchange_handler)(struct backbone_eth* eth);

void add_access_eth_state_handler(access_eth_state_handler h);
void add_backbone_eth_state_handler(backbone_eth_state_handler h);
void add_pkt_rcv_handler(pkt_rcv_handler h);
void add_hello_detection_handler(hello_detection_handler handler);
void add_exchange_handler(exchange_handler h);

#endif /* __LSD_EVENT_H__ */
