#ifndef DBM_H_
#     define DBM_H_

//MacroPart
//
#define SPF_HOLD_TIME 2 //second
//Struct Defined
struct dbm_conf{
	int snapshoot_timeval;
	int policy_type;
	int pma_id;
	int router_id;
};

struct lsdb_msgheader{
	u8 version;
	u8 blank;
	u16 length;
	u32 area_id;
	u8 lsdb_pkt[1];
};
struct link_state_pkt{
	u32 areaid;
	u32 rid;
	u32 nrid;
	u32 ifid;
	u32 nifid;
	u32 metric;
	u32 status;
	u32 seq;
	struct timeval age;
};
typedef struct policyMsg{
	int if_id;//add byMacro
	u32 hello_interval;
	u32 dead_interval;
	u32 retransmit_interval;
}policyMsg;

typedef struct policyMsgHeader{
	char pepversion;
	char operation;
	char msg[0];
} policyMsgHeader;

// Module InitFunc
int dbm_init();
int dbm_start();

// Config Process
int conf_handle(struct   Packet_header *pkt);


// Packet Process
int link_state_handler(struct Packet_header *pkt);
int policy_table_handler(struct Packet_header *pkt);
int spf_signal_handler(struct Packet_header *pkt);
int send_policy(int ifid, u32 hello_timer , u32 dead_timer);

// Self Defination
//

#endif /* DBM_H_ */
