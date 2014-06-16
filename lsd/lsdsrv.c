#include <utils/common.h>
#include <utils/utils.h>
#include <utils/list.h>
#include <lsdb/lsdb.h>
#include <lsd_event.h>
#include <lsd_handler.h>
#include <lsd_if.h>
#include <lsd_socket.h>
#include <lsd_hello.h>
#include <lsd_exchange.h>
#include <lsd_flood.h>

struct thread_master *master;

int lsd_init()
{
	master = thread_master_create();
    lsd_serv_sock();
    add_access_eth_state_handler(ace_up_handler);

	add_backbone_eth_state_handler(hello_on_eth_state_changed);// backbone state changed
#ifndef EXCHANGE_DISABLE
	add_backbone_eth_state_handler(exchange_on_eth_state_changed);// backbone state changed
#endif

#ifndef FLOOD_DISABLE
	add_backbone_eth_state_handler(flood_on_eth_state_changed);// backbone state changed
#endif

    add_pkt_rcv_handler(hello_on_high_packet_received); //high packet received
	add_pkt_rcv_handler(hello_on_low_packet_received); //low packet received

#ifndef EXCHANGE_DISABLE
	add_pkt_rcv_handler(exchange_pkt_received);
#endif

#ifndef FLOOD_DISABLE
	add_pkt_rcv_handler(flood_pkt_received);
#endif
	add_hello_detection_handler(hello_changed_handler);//Hello Changed Handler

}

int lsd_start()
{
    struct thread thread;
    while( thread_fetch(master, &thread))
    {
       thread_call(&thread);
    }
}
