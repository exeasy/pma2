#include <utils/common.h>
#include <header.h>
#include "comm_utils.h"
#include "comm.h"
#include "handler.h"
#include "exterior_daemon.h"
#include "utils/utils.h"
#include "control/control.h"

#define NO_BLOCKED 0
#define BLOCKED 1
struct ctl *data_handle;
struct ctl *capsu_handle;

static int pma_status = NORMAL;

/* get pma status */
int get_pma_status(void)
{
	return pma_status;
}

/* save the current used ip which pma comm with pms */
int pma_addr;
/* get the current used ip */
int get_pma_pms_ip(){
	return pma_addr;
}

/* store the pma's every interface's ipaddress */
struct interface_addr *iplist;

int comm_init(void) //no snapshoot version 
{
	int ret = 0;
	data_handle = create_ctl(1024);
	add_ctl(data_handle, OPS_PMS_PMA_INFO_ACK, handle_ops_ack, 1);
	add_ctl(data_handle, OPS_PMS_PMA_INFO_ERR_ACK, handle_ops_ack_failed, 1);
	add_ctl(data_handle, OPS_PMS_PMA_INIT_REPLY, handle_pma_init_reply, 1);
	//* version 2.0 *//
//	add_ctl(data_handle, OPS_PMS_LOGIN_RESP_OK, handle_ops_pms_login_resp_ok, 1);
//	add_ctl(data_handle, OPS_PMS_LOGIN_FAILED, handle_packet_show_content, 1);
//	add_ctl(data_handle, OPS_ACK_FAILED, handle_ops_ack_failed,1);
//	add_ctl(data_handle, OPS_ACK, handle_ops_ack,1);
	//* version 2.0 *//
//	capsu_handle = create_ctl(10);
//	add_ctl(capsu_handle, OPS_PMA_TRANS_PACKET, encapsulate_trans_packet, 1);
//	add_ctl(capsu_handle, OPS_PMA_LOGIN, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, OPS_PMA_POLICY_REQ, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, OPS_PMA_NET_INFO_SEND, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, OPS_PMA_SNAPSHOT_SEND, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, OPS_PMA_LOGGOUT, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, PMA_PMS_PULSE_INFO, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, PMA_CREATE_MANAGED_INFO, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, TC_TO_PMA_INTERFACE_INFO, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, BGP_LINK_INFO, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, TC_TO_PMA_FLOW_INFO, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, TC_CMD_ACK_TO_PMS, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, OPS_OPENFLOW_FLOW_INFO_SEND, encapsulate_normal_packet, 1);
//	add_ctl(capsu_handle, PKT_TYPE_EVENT_NOTIFY, encapsulate_normal_packet, 1);

	get_interface_ip();
	ret = exterior_daemon_init();
	if (ret != 0) {
		DEBUG(ERROR, "daemon_init failed %s", strerror(errno));
		return -1;
	}
	return 0;
}

int update_pma_addr(int sockfd){
	struct sockaddr_in client_addr;
	int len = sizeof(client_addr);
	char pmaip[24];
	getsockname(sockfd,&client_addr,&len);
	pma_addr = client_addr.sin_addr.s_addr;
	inet_ntop(AF_INET,&client_addr.sin_addr,pmaip,24);
	DEBUG(INFO,"Client Used IP[%s] to PMS",pmaip);
}

int create_connect(char *ip, int port)
{
	struct sockaddr_in server_addr;
	int server_sock;
	int connected = 0;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);
	int serv_len = sizeof(struct sockaddr_in);
	DEBUG(INFO,"Trying to connect the Server...");
	int ret = 0;int n;
	server_sock =  socket(AF_INET,SOCK_STREAM,0);
	make_socket_status(server_sock,NO_BLOCKED);
	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));//make the socket reuse able 
	if(server_sock <0)
	{
		DEBUG(ERROR,"Cant create socket");
		close(server_sock);
		return -1;
	}
	ret = connect(server_sock,&server_addr,sizeof(struct sockaddr_in));
	if(ret != 0)
	{
		if(errno != EINPROGRESS)
		{
			DEBUG(ERROR,"Connect error");
			close(server_sock);return -1;
		}
		else
		{
			struct timeval tm;
			int timeout = CONNECT_TIMEOUT;

			tm.tv_sec = timeout/1000;
			tm.tv_usec = timeout%1000 * (1000);

			fd_set set ,rset;
			FD_ZERO(&set);
			FD_ZERO(&rset);
			FD_SET(server_sock,&set);
			FD_SET(server_sock,&rset);
			socklen_t len;
			int error = -1;
			int res;
			res = select(server_sock+1,&rset,&set,NULL,&tm);
			if(res < 0)
			{
				close(server_sock);
				DEBUG(ERROR,"Connect error");
				return -1;
			}
			else if(res == 0)
			{
				close(server_sock);
				DEBUG(ERROR,"Connect time out");
				return -1;
			}
			else
			{
				if(FD_ISSET(server_sock,&set)&&!FD_ISSET(server_sock, &rset))
				{
					DEBUG(INFO,"connect success");
					make_socket_status(server_sock,BLOCKED);
					return server_sock;
				}
				else
				{
					close(server_sock);
					DEBUG(ERROR,"Connect Failed");
					return -1;
				}
			}
		}
	}
	else
	{
		connected = 1;
		DEBUG(INFO,"Connect success");
		make_socket_status(server_sock,BLOCKED);
		return server_sock;
	}
}

int close_connect(int sockfd)
{
	return (close(sockfd));
}

int encapsulate_trans_packet(struct packet *pkt)
{
	struct pma_pms_header header;
	int len = sizeof(struct pma_pms_header);
	pkt->len += len;
	pkt->len -= sizeof(u8);
	char type;
	memcpy(&type,pkt->data,sizeof(u8));
	header.pph_pkg_type = (u8)type;
	header.pph_pkg_version = get_version();
	header.pph_pkg_dev_type = htons(DEVICETYPE_APP_BM);
	header.pph_pkg_len = htonl(pkt->len);
	header.pph_agent_id = htonl(get_pma_id());
	int saddr = get_pma_pms_ip();//get_local_ip();
	header.src_addr.s_addr = saddr;
	inet_pton(AF_INET,pkt->ip,&header.dst_addr);
	char* tmp = pkt->data + sizeof(u8);
	printf("\nData:%s\n",tmp);
	pkt->data = pkt->data - len + sizeof(u8);
	memcpy(pkt->data, (void *)&header, len);
	return 0;
}
int encapsulate_normal_packet(struct packet *pkt)
{
	struct pma_pms_header header;
	int len = sizeof(struct pma_pms_header);
	pkt->len += len;

	header.pph_pkg_type = pkt->ops_type;
	header.pph_pkg_version = get_version();
	header.pph_pkg_dev_type = htons(DEVICETYPE_APP_BM);
	header.pph_pkg_len = htonl(pkt->len);
	header.pph_agent_id = htonl(get_pma_id());
	int saddr = get_pma_pms_ip();//get_local_ip();
	header.src_addr.s_addr = saddr;
	inet_pton(AF_INET,pkt->ip,&header.dst_addr);

	pkt->data = pkt->data - len;
	memcpy(pkt->data, (void *)&header, len);
	return 0;
}
int encapsulate_packet(struct packet *pkt, void* buf)
{
	DEBUG(INFO, "%s %d %d %u encapsulate_packet handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);

	if (buf != NULL) {
		pkt->head = (char *)malloc(MAX_PACKET_HEADER_SIZE + pkt->len);
		pkt->data = pkt->tail = pkt->end
			= (pkt->head + MAX_PACKET_HEADER_SIZE + pkt->len - 1);
		pkt->data = pkt->tail - pkt->len;
		memcpy(pkt->data, buf, pkt->len);
		*(pkt->tail) = 0;
	} else {
		pkt->head = (char *)malloc(MAX_PACKET_HEADER_SIZE);
		pkt->data = pkt->tail = pkt->end
			= (pkt->head + MAX_PACKET_HEADER_SIZE - 1);
		pkt->len = 0;
	}
	encapsulate_normal_packet(pkt);
	return 0;
//	run_ctl(capsu_handle, pkt->ops_type , pkt);
//	switch (pkt->ops_type){
//		case OPS_PMA_TRANS_PACKET://add by macro
//			{
//				struct pma_pms_header header;
//				int len = sizeof(struct pma_pms_header);
//				pkt->len += len;
//				pkt->len -= sizeof(u8);
//				char type;
//				memcpy(&type,pkt->data,sizeof(u8));
//				header.pph_pkg_type = (u8)type;
//				header.pph_pkg_version = get_version();
//				header.pph_pkg_dev_type = htons(DEVICETYPE_APP_BM);
//				header.pph_pkg_len = htonl(pkt->len);
//				header.pph_agent_id = htonl(get_pma_id());
//				int saddr = get_pma_pms_ip();//get_local_ip();
//				header.src_addr.s_addr = saddr;
//				inet_pton(AF_INET,pkt->ip,&header.dst_addr);
//				char* tmp = pkt->data + sizeof(u8);
//				printf("\nData:%s\n",tmp);
//				pkt->data = pkt->data - len + sizeof(u8);
//				memcpy(pkt->data, (void *)&header, len);
//			}break;
//		case OPS_PMA_LOGIN:
//		case OPS_PMA_POLICY_REQ:
//		case OPS_PMA_NET_INFO_SEND:
//		case OPS_PMA_SNAPSHOT_SEND:
//		case OPS_PMA_LOGGOUT:
//		case PMA_PMS_PULSE_INFO:
//		case PMA_CREATE_MANAGED_INFO:
//		case TC_TO_PMA_INTERFACE_INFO:
//		case BGP_LINK_INFO:
//		case TC_TO_PMA_FLOW_INFO:
//		case TC_CMD_ACK_TO_PMS:
//		case OPS_OPENFLOW_FLOW_INFO_SEND:
//		case PKT_TYPE_EVENT_NOTIFY:
//			{
//				struct pma_pms_header header;
//				int len = sizeof(struct pma_pms_header);
//				pkt->len += len;
//
//				header.pph_pkg_type = pkt->ops_type;
//				header.pph_pkg_version = get_version();
//				header.pph_pkg_dev_type = htons(DEVICETYPE_APP_BM);
//				header.pph_pkg_len = htonl(pkt->len);
//				header.pph_agent_id = htonl(get_pma_id());
//				int saddr = get_pma_pms_ip();//get_local_ip();
//				header.src_addr.s_addr = saddr;
//				inet_pton(AF_INET,pkt->ip,&header.dst_addr);
//
//				pkt->data = pkt->data - len;
//				memcpy(pkt->data, (void *)&header, len);
//			}
//			return 0;
//		default:
//			{
//				DEBUG(INFO, "client: handle_message: can't recognize the operation.");
//				return -1;
//			}
//	}
}
int send_packet(struct packet *pkt)
{
	DEBUG(INFO, "%s %d %d %u send_packet handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	int ret;
	ret = send(pkt->sockfd, pkt->data, pkt->len,0);
	if (ret == -1) {
		DEBUG(ERROR, "%s %d %d %u send_packet send failed :%s",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len, strerror(errno));
		return -1;
	}
	DEBUG(INFO, "%s %d %d %u %u send_packet send",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len, ret);
	return ret;
}

int send_packet_by_alca(struct packet*pkt)
{
	DEBUG(INFO, "%s %d %d %u ALCA send_packet handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
	int ret;
	int pkt_len = pkt->len+sizeof(struct alca_header);
	struct alca_header* packet = (struct alca_header*)malloc(pkt_len);
	inet_pton(AF_INET, pkt->ip,&packet->dst_ip);

	packet->pkg_type = PKT_TYPE_TCP;
	packet->pkg_version = ALCA_VERSION;
	packet->pkt_len = htonl(pkt_len);
	packet->port = htons(pkt->port);
	packet->max_delay = 0;
	packet->reserved = 0;
	printf("%s %d %d\n",pkt->ip,pkt_len,pkt->port);

	memcpy((char*)packet+sizeof(struct alca_header),pkt->data,pkt->len);
	ret = send(pkt->sockfd, packet,pkt_len,0);
	if (ret == -1) {
		DEBUG(ERROR, "%s %d %d %u send_packet send failed :%s",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len, strerror(errno));
		return -1;
	}
	DEBUG(INFO, "%s %d %d %u %u send_packet send",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len, ret);

	return ret;
}


int recv_packet(struct packet *pkt,int old_type)
{
	DEBUG(INFO, "%s %d %d %u recv_packet handle",pkt->ip, pkt->sockfd, pkt->ops_type, 0);

	pkt->len = pkt->end - pkt->head;
	pkt->data = pkt->tail = pkt->head;
	memset(pkt->data, 0, pkt->len);
	char *newbuf = pkt->data;

	fd_set rd;
	FD_ZERO(&rd);
	FD_SET(pkt->sockfd, &rd);
	int nfds = pkt->sockfd + 1;

	int ret = 0;

	struct timespec t;
	t.tv_sec = pkt->timeout;
	t.tv_nsec =0;

	while (1) {
		ret = pselect(nfds,&rd,NULL,NULL,&t,NULL);

		if (ret == -1 && errno == EINTR) {
			DEBUG(ERROR,"a signal interrupt");
			continue;
		}

		if (ret == -1) {
			DEBUG(ERROR, "%s %d %d %u recv_packet handle pselect %s",pkt->ip, pkt->sockfd, pkt->ops_type, 0, strerror(errno));
			return -1;
		}

		if (ret == 0) {
			DEBUG(ERROR, "%s %d %d %u recv_packet handle timeout",pkt->ip, pkt->sockfd, pkt->ops_type, 0);
			return -1;
		}

		u32 numbytes = recv(pkt->sockfd, pkt->data, pkt->len, 0);

		if (numbytes == -1) {
			DEBUG(ERROR, "%s %d %d %u recv_packet handle recv %s",pkt->ip, pkt->sockfd, pkt->ops_type, numbytes,strerror(errno));
			return -1;
		} else if (numbytes == 0) {
			DEBUG(ERROR, "%s %d %d %u recv_packet handle :the connection has been closed.",pkt->ip, pkt->sockfd, pkt->ops_type, numbytes);
			return -1;
		}
		DEBUG(INFO, "%s %d %d %u %u recv_packet handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len, numbytes);


		u32 len;


		struct common_header * header = (struct common_header *)pkt->data;
		len = ntohl(header->pkg_len);
		pkt->ops_type = header->pkg_type;
		pkt->len = len;
		// pkt->ops_type = header->pkg_type;

		if (len > numbytes) {
			newbuf = (char *)malloc(len+1);
			if (newbuf == NULL) {
				DEBUG(INFO, "%s %d %d %u %u recv_packet handle malloc %s",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len, numbytes,strerror(errno));
				return -1;
			}
			char *cur_pos =  newbuf;
			memcpy(newbuf, pkt->data, numbytes);
			cur_pos += numbytes;
			len -= numbytes;
			//dont forget test this part
			while(len > 0){
				numbytes = recv(pkt->sockfd, cur_pos, len, 0);
				if (numbytes == -1) {
					if (errno == EAGAIN || errno == EWOULDBLOCK){
						DEBUG(ERROR, "%s %d %d %u (errno == EAGAIN || errno == EWOULDBLOCK) %s",pkt->ip, pkt->sockfd, pkt->ops_type, 0, strerror(errno));
						continue;
					}
					free_buf(newbuf);
					DEBUG(ERROR, "%s %d %d %u recv_packet handle pselect %s",pkt->ip, pkt->sockfd, pkt->ops_type, 0, strerror(errno));
					return -1;
				} else if (numbytes == 0) {
					DEBUG(ERROR, "%s %d %d %u recv_packet handle :the connection has been closed.",pkt->ip, pkt->sockfd, pkt->ops_type, numbytes);
					free_buf(newbuf);
					return -1;
				}
				DEBUG(INFO, "%s %d %d %u %u recv_packet handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len, numbytes);
				cur_pos += numbytes;
				len -= numbytes;
			}
			*(cur_pos) = 0;

			free_buf(pkt->data);
			pkt->head = pkt->data = newbuf;
			pkt->end = pkt->tail = cur_pos;
		}

		DEBUG(INFO, "%s %d %d %u recv_packet handle length",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);
		return 0;
	}
	return 0;
}

int send_message_to_pms(char *ip, int port, int type, char *buf, int len)
{
	int ret = 0,sockfd;
	int flag = 0;

	char* alca_ip = get_alca_address();
	int alca_port = get_alca_port();
	if(get_comm_type()==2)
	{
		sockfd = create_connect(alca_ip,alca_port);

	}
	else 
		sockfd = create_connect(ip, port);




	if (sockfd == -1){
		DEBUG(ERROR, "%s %d create_connect", ip, type);
		if(get_comm_type()==1)//Enable ALCA
		{

			DEBUG(INFO,"Trying ALCA ING.............");

			sockfd = create_connect(alca_ip,alca_port);
			if (sockfd == -1){
				DEBUG(ERROR, "%s %d ALCA create_connect", ip, type);
				if(get_pma_status() != HOSTED)
					return -1;
			}
			flag =1;
		}
		else if(get_comm_type()==0){
			if(get_pma_status() != HOSTED)
				return -1;
		}
	}
	else
	{
		update_pma_addr(sockfd);
	}
	DEBUG(INFO, "%s %d %d send_data", ip, type,sockfd);
	if (buf != NULL) {

	}
	struct packet pkt;
	memset(&pkt, 0, sizeof(pkt));
	memcpy(pkt.ip, ip, strlen(ip));
	pkt.port = port;
	pkt.sockfd = sockfd;
	pkt.len = len;
	pkt.timeout = 10;	//default 10s
	pkt.ops_type = type;

	ret = encapsulate_packet(&pkt, buf);

	if (ret == -1){
		DEBUG(ERROR, "%s %d %d encapsulate_packet", pkt.ip, pkt.sockfd,pkt.ops_type);
		close_connect(sockfd);
		return -1;
	}



	if(2 == get_comm_type()||flag == 1)
	{
		ret = send_packet_by_alca(&pkt);
		if(ret == -1)
		{
			DEBUG(ERROR, "%s %d %d send_packet", pkt.ip, pkt.sockfd,pkt.ops_type);
			free_buf(pkt.head);
			close_connect(sockfd);
			return -1;
		}
	}
	else{
		ret = send_packet(&pkt);
		if (ret == -1){
			DEBUG(ERROR, "%s %d %d send_packet", pkt.ip, pkt.sockfd,pkt.ops_type);
			if(1 == get_comm_type()){
				ret = send_packet_by_alca(&pkt);
				if(ret == -1)
				{
					DEBUG(ERROR, "%s %d %d send_packet", pkt.ip, pkt.sockfd,pkt.ops_type);
					free_buf(pkt.head);
					close_connect(sockfd);
					return -1;
				}
			}
			else if(0 == get_comm_type())
			{
				free_buf(pkt.head);
				close_connect(sockfd);
				return -1;	
			}
		}

	}
	pkt.timeout = get_timeval(); 
	ret = recv_packet(&pkt,0);
	if (ret == -1){
		DEBUG(ERROR, "%s %d %d recv_packet", pkt.ip, pkt.sockfd,pkt.ops_type);
		free_buf(pkt.head);
		close_connect(sockfd);
		return -1;
	}

	ret = pms_reply_handle(&pkt);
	if (ret == -1){
		DEBUG(ERROR, "%s %d %d pms's reply handle", pkt.ip, pkt.sockfd,pkt.ops_type);
		free_buf(pkt.head);
		close_connect(sockfd);
		return -1;
	}
	close_connect(sockfd);

	return 0;
}


int pms_reply_handle(struct packet *pkt)
{
	DEBUG(INFO, "%s %d %d %u pms reply handle",pkt->ip, pkt->sockfd, pkt->ops_type, pkt->len);

	struct pma_pms_header *header = (struct pma_pms_header *)pkt->data;

	return run_ctl( data_handle, pkt->ops_type , pkt);
}
int show_interface()
{
	struct interface_addr *tmp = iplist->next;
	int i = 0;
	while(tmp != NULL)
	{
		char ip[24];
		inet_ntop(AF_INET, &tmp->addr.sin_addr, ip , 24);
		DEBUG(INFO,"Interface%d IP:%s",++i, ip);
		tmp = tmp->next;
	}
	return 0;
}	


int get_interface_ip()
{
	iplist = (struct interface_addr*)malloc(sizeof(struct interface_addr));
	iplist->next = NULL;
	//get all the interface ip try it one by one
	struct interface_addr* handle = iplist;
	int tmpsock = socket(AF_INET,SOCK_STREAM,0);
	if(tmpsock <0)
	{
		DEBUG(INFO,"create socket failed");
		return -1;
	}
	int i;
	struct ifconf ifconf;
	struct ifreq* ifreq;
	unsigned char buf[512];
	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;
	ioctl(tmpsock,SIOCGIFCONF,&ifconf);//get all the ip
	ifreq = (struct ifreq*)buf;
	for(i=(ifconf.ifc_len/sizeof(struct ifreq));i>0;i--)
	{
		ifreq++;
		handle = (struct interface_addr*)malloc(sizeof(struct interface_addr));
		memcpy(&handle->addr,(struct sockaddr_in*)(&ifreq->ifr_addr),sizeof(struct sockaddr_in));
		//handle->next = NULL;
		struct interface_addr* tmp = iplist->next;
		iplist->next = handle;
		handle->next = tmp;
	}
}

int make_socket_status( int sfd, int block)
{
	int flags, s;

	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror ("fcntl");
		return -1;
	}
	if(block == NO_BLOCKED)
		flags |= O_NONBLOCK;
	else if(block == BLOCKED)
		flags &= ~O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror ("fcntl");
		return -1;
	}

	return 0;
}
