#include <stdio.h>
#include <lib/event.h>
#include <lsd_cmn.h>
#include "lsd_event.h"


define_event_queue(access_queue);
define_event_queue(backbone_queue);
define_event_queue(pkt_rcv_queue);
define_event_queue(detection_queue);
define_event_queue(exchange_queue);

void add_access_eth_state_handler(access_eth_state_handler h)
{
	add_event_handler(&access_queue,h);
}
void add_backbone_eth_state_handler(backbone_eth_state_handler h)
{
	add_event_handler(&backbone_queue, h);
}
void add_pkt_rcv_handler(pkt_rcv_handler h)
{
	add_event_handler(&pkt_rcv_queue, h);
}
void add_hello_detection_handler(hello_detection_handler handler)
{
	add_event_handler(&detection_queue, handler);
}
void add_exchange_handler(exchange_handler h)
{
	add_event_handler(&exchange_queue, h);
}
