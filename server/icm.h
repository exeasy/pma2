#ifndef ICM_H_
#     define ICM_H_

//MacroPart
//

//Struct Defined
struct icm_conf{
	struct in_addr router_ip;
	struct in_addr local_ip;
	struct in_addr netmask;
	int outside;
	int device_type;
	int pma_id;
	int if_id; //-1 present all the interface 
	unsigned int h_hello_val;
	unsigned int h_dead_val;
	unsigned int l_hello_val;
	unsigned int l_dead_val;	
};

struct pma_list{
	int device_id;
	int router_id;
	struct in_addr ip;
};


extern struct ctl *packet_handler;

// Module InitFunc
int icm_init();
int icm_start();

// Config Process
int conf_handle(struct Packet_header *pkt);

// Packet Process
int pma_list_handle(struct Packet_header *pkt);

// Self Defination
//

int route_table_daemon();
int ifinfo_daemon();
int spf_count_daemon();
#endif /* ICM_H_ */
