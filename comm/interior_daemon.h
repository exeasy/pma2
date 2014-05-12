#ifndef INTERIOR_DAEMON_H
#define INTERIOR_DAEMON_H

#define MAXBUF 50000

#define PMA_VERSION 3

#define ICM		1
#define DBM		2
#define PEM		3
#define BM		4
#define PMS		5

/* pkt type by Macro.Z 2013-01-23 begin */
#define MODREG 1
#define EVENTREG 2
#define HEARTPKT 3
#define ACK 4
/*Macro.Z 2013-01-23 end*/


/* IC module's actions by Macro.Z 2013-01-23 begin */
#define ADDLSA 11
#define UPDATELSA 12
#define ADDROUTE 13
#define UPDATEROUTE 14
#define UP_NETWORK_INFO 15
#define UP_ROUTE_INFO 16
#define UP_INTERFACE_INFO 17
/*Macro.Z 2013-01-23 end*/


/* DBM's actions by Macro.Z 2013-01-23 begin */
#define NETWORK_INFO 21
#define LSDB_INFO	22
#define POLICY_INFO 231//for test
/*Macro.Z 2013-01-23 end*/

/* PEA's actions by Macro.Z 2013-03-15 begin */
#define OSPF_SPF 31
#define POLICY_REQUEST 32
/*Macro.Z 2013-03-15 end*/


/* BM's actions by Macro.Z 2013-03-15 begin */
#define POLICY_LIST  51
#define NETINFO_REQ 52
#define SNAPSHOOT_REQ 53
#define PMA_LIST 54
#define ICM_CONF 55
#define DBM_CONF 56
#define PEM_CONF 57

/*Macro.Z 2013-03-15 end*/



/*TCM 2013-04-15 begin*/
#define TC_TO_PMA_INTERFACE_INFO 23
#define TC_TO_PMA_FLOW_INFO 24
/*TCM 2013-04-15 end*/

#define BM_STATUS_ALL_READY 3  //module:ic+dbm+pea
#define BM_STATUS_TINY_READY 2 //module:ic+pea
#define BM_STATUS_NOT_READY 1 //module:other
#define BM_STATUS_CLOSED 0 //only the basemodule
#define BGP_MRAI    60

#define BUFFLEN 1024  //not used
#define SERVER_PORT "8888"
#define BACKLOG 5  //not used
#define CLIENTNUM 1024
#define MAXEVENTS 64

struct pkt{
	char* data;
	int fd;
};

typedef struct Packet_header{
	u8 pkt_type;
	u8 pkt_version;
	u16 pkt_len;
	u32 checksum;
	u8 pkt[0];
} local_module_header;

//应答消息
struct response_message
{
	int des_mod;
	int ret;		//返回值 0表示成功 其他表示失败
	char info[100];		//返回信息
};


/* gloal buff by Macro.Z 2013-03-27 begin */
struct global_buff{
	char* buff;
	int length;
};

extern char * modlist[];
extern struct global_buff xml_policy_buff;
extern struct global_buff global_pma_buff;
extern int bm_status;
int interior_daemon_init();
int send_pkt(int sock,local_module_header* pkt,int length);
int send_message_to_module( int module , int pkg_type , char* data, int length);



#endif

