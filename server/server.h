#ifndef PMA_SERVER_H
#define PMA_SERVER_H

declare_event_queue_list(module_transfer, M_STATUS_TOTAL);

int module_start();
int module_init();
int module_register();
int module_daemon();
int module_get_data();
int module_send_data(char *data, int length, int type);
int module_set_status(int status);

extern int ack_handle(struct Packet_header* pkt);
#endif
