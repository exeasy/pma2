#ifndef PMA_SOCKET_H
#define PMA_SOCKET_H

#define PMA_BM_ADDR "127.0.0.1"
#define PMA_BM_PORT 8888

#define DISCONNECT	0
#define CONNECT		1

typedef struct Packet_header{
	u8 pkt_type;
	u8 pkt_version;
	u16 pkt_len;
	u32 checksum;
	u8 pkt[0];
} Packet_header, *Packet_headerp;

struct response_message
{
	int des_mod;
	int ret;		//返回值 0表示成功 其他表示失败
	char info[100];		//返回信息
};

extern int con_status;
extern int module_status;

extern int get_bm_sock();

#endif
