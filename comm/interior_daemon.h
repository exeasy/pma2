#ifndef INTERIOR_DAEMON_H
#define INTERIOR_DAEMON_H

#define MAXBUF 50000


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

