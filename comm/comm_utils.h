#ifndef PMA_COMM_UTILS_H
#define PMA_COMM_UTILS_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define MAXBUF 1024*50

struct common_header{
	u8 pkg_type;
	u8 pkg_version;
	u16 device_type;
	u32 pkg_len;
};

struct pma_pms_header{
	struct common_header com_h;
#define pph_pkg_type com_h.pkg_type
#define pph_pkg_version com_h.pkg_version
#define pph_pkg_dev_type com_h.device_type
#define pph_pkg_len com_h.pkg_len
	u32 pph_agent_id;
	struct in_addr src_addr;
	struct in_addr dst_addr;
};

struct pma_inner_header{
	struct common_header com_h;
#define pch_pkg_type com_h.pkg_type
#define pch_pkg_type com_h.pkg_version
#define pch_pkg_len com_h.pkg_len
	u32 pch_rt_id;
};



#define MAX_PACKET_HEADER_SIZE (sizeof(struct pma_pms_header ) + 10)

struct packet{
	char	ip[INET_ADDRSTRLEN];
	int		port;
	int		sockfd;
	unsigned int	len;
	int		timeout;
	int		ops_type;
	char	*head;
	char	*data;
	char	*tail;
	char	*end;
};

struct alca_header{
	u8 pkg_type; /*!< 包类型*/
	u8 pkg_version; /*!< 包版本*/
	u16 port; /*!<目的端口*/
	u32 dst_ip; /*!< 目地IP */
	u32 pkt_len; //包总长度
	u32 max_delay; //最大延迟
	u32 reserved; //保留字段
};


int setnonblocking(int sockfd);

int setblocking(int sockfd);

void free_buf(void *buf);

char *get_peer_ip(int fd);

u16 get_peer_port(int fd);

#endif


