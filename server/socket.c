#include <utils/common.h>
#include <utils/utils.h>
#include <srvconf.h>
#include <socket.h>

unsigned int global_bm_sock;
int get_bm_sock(){return global_bm_sock;}
int con_status;
int module_status;

int srv_connect()
{
	int ret;
	signal(SIGPIPE, SIG_IGN);

	struct sockaddr_in srv_addr;
	ret = socket(AF_INET, SOCK_STREAM, 0 );
	if( ret < 0 )
	{
		DEBUG(INFO, "can't create socket");
		return -1;
	}
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr(PMA_BM_ADDR);
	srv_addr.sin_port = htons(PMA_BM_PORT);
	int srv_len = sizeof( struct sockaddr_in );

	if(connect(ret,(struct sockaddr*)&srv_addr,srv_len)<0)
	{
		DEBUG(INFO,"connect error");
		return -1;
	}
	DEBUG(INFO,"successfully connected!");
	global_bm_sock = ret;
	con_status = CONNECT;
	return 0;
}

int send_packet(Packet_header* pkt)
{
	int ret;
	int bm_sock = get_bm_sock();
	if(bm_sock == 0 ){
		DEBUG(ERROR,"There is no socket fd");
		return -1;  
	}
	int pkt_len = pkt->pkt_len;
	ret = send(bm_sock,pkt, pkt_len,0);
	if (ret == -1) {
		if(errno == SIGPIPE)
		{
			con_status = DISCONNECT;
			module_status = M_STATUS_OFFLINE;
		}
		DEBUG(ERROR, "send_packet send failed :type:[%d] --%s",pkt->pkt_type, strerror(errno));
		return -1;
	}
	DEBUG(INFO, "%d %d %u %u send_packet send", global_bm_sock,pkt->pkt_type, pkt->pkt_len, ret);
	free(pkt);

	return ret;
}

struct Packet_header* recv_packet(){
	int ret;
	Packet_header header;
	int bm_sock = get_bm_sock();
	ret = recv(bm_sock,&header,sizeof(Packet_header),0); 
	if(ret<0)
	{
		return NULL;
	}
	if(ret==0)
	{
		con_status=DISCONNECT;
		return NULL;
	}
	int pkt_len = header.pkt_len;
	Packet_header* pkt = (Packet_header*)malloc(pkt_len);
	pkt->pkt_type = header.pkt_type;
	pkt->pkt_version = header.pkt_version;
	pkt->pkt_len = header.pkt_len;

	if(pkt->pkt_len < sizeof(Packet_header))
	{
		return NULL;
	}
	if(pkt->pkt_len == sizeof(Packet_header))
	{
		return pkt;
	}
	ret = recv(bm_sock,(void*)pkt->pkt,pkt_len-sizeof(Packet_header),0);

	if(ret<0)
	{
		return NULL;
	}
	if(ret==0)
	{
		con_status=DISCONNECT;
		return NULL;
	}
	return pkt;
}


