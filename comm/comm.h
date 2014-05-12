#ifndef PMA_COMM_H
#define PMA_COMM_H


#define CONNECT_TIMEOUT 500 //ms 500ms is enough

// Store every interface's address
struct interface_addr{
	struct sockaddr_in addr;
	struct interface_addr *next;
};

struct req_args{
	char ip[46];
	int port;
	int ops_type;
	int len;
	char data[0];
};

extern struct interface_addr *iplist;// iplist present the interface's addresses.


int comm_init(void); //no snapshoot version 
// Part 1. Connection
int create_connect(char *ip, int port);
int close_connect(int sockfd);
int update_pma_addr(int sockfd);
// Part 2. Packet Packing
int encapsulate_trans_packet(struct packet *pkt);
int encapsulate_normal_packet(struct packet *pkt);
int encapsulate_packet(struct packet *pkt, void* buf);
// Part 3. Packet Send & Recv
int send_packet(struct packet *pkt);
int recv_packet(struct packet *pkt, int old_type);
int send_data(char *ip, int port, int type, char *buf, int len);
// Part 4. Packet Handle
int handle_data(struct packet *pkt);
// Part 5. Interface Process
int show_interface();
int get_interface_ip();

#endif
