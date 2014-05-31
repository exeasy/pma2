#ifndef MODULE_PEM_H
#define MODULE_PEM_H

#define TIMEPART 1000000
#define MAX_INTERFACE 100

//LOGGER LEVEL
 

//Config Defination
struct pem_conf{
	struct in_addr router_ip;
	int pma_id;
};
struct bgp_interface{
    int ifid;
    char name[24];
    char neighbor_ip[24];
    char local_ip[24];
    int remote_as;
    struct bgp_interface* next;
};



typedef struct policyMsg{
	int if_id;//add byMacro
	unsigned long hello_interval;
	unsigned long  dead_interval;
	unsigned long restransmit_interval;
}policyMsg;

typedef struct policyMsgHeader{
	char pepversion;
	char operation;
	char msg[0];
} policyMsgHeader;


extern struct ctl *packet_handler;

//Module InitFunc
int pem_init();

//Config process
int conf_handle(struct Packet_header* pkt);

//Packet Process
int policy_handle(struct Packet_header* pkt);
int mrai_handle(struct Packet_header* pkt);
int tunnel_command_handle(struct Packet_header* pkt);

//Self Defination
int init_interface_list();

#endif
