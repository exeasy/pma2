#include <utils/common.h>
#include <utils/utils.h>
#include "exterior_daemon.h"
#include "comm_utils.h"
#include "header.h"
#include "conf/conf.h"
#include "control/control.h"
#include "handler.h"

static void *exterior_daemon(void *args);

static void listener(int efd, int sfd);

static void *packet_handle(void *args);

struct ctl *upload_handle;

int exterior_daemon_init(void)
{
	init_ctls();
	upload_handle = create_ctl(1024);
	add_ctl(upload_handle, OPS_PMS_PMA_POLICY_INFO,   handle_ops_pms_policy_send,		1);
	add_ctl(upload_handle, OPS_PMS_MARI_SEND,         handle_ops_pms_mari_send,			1);
	add_ctl(upload_handle, OPS_PMS_PMA_NEIGHBOR_INFO, handle_ops_pms_neighbor_list,		1);
	add_ctl(upload_handle, OPS_TUNNEL_COMMAND,        handle_ops_pms_tun_command,		1);

	//add_ctl(upload_handle, TC_CMD_PMS_TO_PEA,         handle_ops_pms_tc_cmd,			1);
	add_ctl(upload_handle, OPS_PMA_ROUTER_CONF_INFO,  handle_trans_message,		1);
	add_ctl(upload_handle, OPS_PMA_OSPF_CONF_INFO,    handle_trans_message,		1);
	add_ctl(upload_handle, OPS_PMA_BGP_CONF_INFO,     handle_trans_message,		1);

	add_ctl(upload_handle, BGP_TO_PMA_UPDATE_MESSAGE, handle_trans_message,		1);
	add_ctl(upload_handle, BGP_TO_PMA_RIB,            handle_trans_message,		1);
	add_ctl(upload_handle, BGP_TO_PMA_CONF,           handle_trans_message,		1);
	add_ctl(upload_handle, BGP_NEIGHBOR,              handle_trans_message,		1);
	add_ctl(upload_handle, OPS_PMA_SNAPSHOT_SEND,     handle_trans_message,		1);
	add_ctl(upload_handle, OPS_PMA_TRANS_PACKET,      handle_trans_message,		1);


	pthread_t comm_thread;
	void *args = NULL;
	return pthread_create(&comm_thread, NULL, exterior_daemon, args);
}

static void *exterior_daemon(void *args)
{
	int sfd;
	struct addrinfo hints, *servinfo, *p;

	int rv;

	memset(&hints, 0 , sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_next = NULL;

	char port[8] = {0};
	sprintf(port, "%d", get_listen_port());

	rv = getaddrinfo(NULL, port , &hints, &servinfo);
	if(rv != 0)
	{
		DEBUG(ERROR, "getaddrinfo: %s",gai_strerror(rv));
		exit(EXIT_FAILURE);
	}

	int flag = 1;
	struct linger ling = {0,0};
	for( p = servinfo ; p != NULL; p = p->ai_next)
	{
		if (( sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1 )
		{
			DEBUG(ERROR, "socket(): %s", strerror(errno));
			continue;
		}
		if (setsockopt( sfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int))
				== -1 )
		{
			close(sfd);
			DEBUG(ERROR, "setsockopt(SO_REUSEADDR): %s", strerror(errno));
			continue;
		}
		if (setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(int))
				== -1) {
			close(sfd);
			DEBUG(ERROR, "setsocketopt(SO_KEEPALIVE): %s", strerror(errno));
			continue;
		}
		if (setsockopt(sfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling))
				== -1) {
			close(sfd);
			DEBUG(ERROR, "setsocketopt(SO_LINGER): %s", strerror(errno));
			continue;
		}
		if (bind(sfd, p->ai_addr, p->ai_addrlen)
				== -1) {
			close(sfd);
			DEBUG(ERROR, "bind: %s", strerror(errno));
			continue;
		}
		break;
	}
	if (p == NULL) {
		DEBUG(ERROR, "the daemon failed to bind any local address, the program will terminate.");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(servinfo);
	if (listen(sfd, BACKLOG) == -1) {
		close(sfd);
		DEBUG(ERROR, "listen(): %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	DEBUG(INFO, "daemon waiting for connections...");

	listener(0, sfd);
	return NULL;
}

static void listener(int efd, int sfd)
{
	//client socket file descriptor
	int cfd;
	//store the client address
	struct sockaddr_storage client_addr;
	//the size of client address
	size_t sin_size = sizeof (struct sockaddr_storage);

	while (1) {
		cfd = accept(sfd, (struct sockaddr *) &client_addr,&sin_size);
		if (cfd < 0) {
			DEBUG(ERROR, "accept");
			continue;
		} else {
			DEBUG(INFO, "%s - %d - %d", get_peer_ip(cfd),ntohs(((struct sockaddr_in *)&client_addr)->sin_port), cfd);
		}

		pthread_t  comm_thread;
		void *args = malloc(sizeof(int));
		if (args != NULL){
			memset(args, 0 ,sizeof(int));
			memcpy(args,&cfd, sizeof(int));
			if (pthread_create(&comm_thread,NULL,packet_handle,args) == 0){
				pthread_detach(comm_thread);
			}
		}
	}
	//never reach this place
	close(sfd);
}

static void *packet_handle(void *args)
{
	DEBUG(INFO,"This is PMA Message Handle Function");
	int fd = 0;
	memcpy(&fd, args, sizeof(int));
	free(args);
	DEBUG(INFO,"fd = %d",fd);
	u32 numbytes;
	char buf[MAXBUF] = {0};
	char *newbuf = buf;
	struct packet pkt;
	memset(pkt.ip, 0, sizeof(pkt.ip));
	memcpy(pkt.ip, get_peer_ip(fd), sizeof(pkt.ip));
	pkt.port = get_peer_port(fd);
	pkt.sockfd = fd;

	if ((numbytes = recv(fd, buf, MAXBUF, 0)) == -1) {
		close(fd);
		DEBUG(ERROR, "packet_handle recv: %s", strerror(errno));
		return PTR_ERR;
	} else if (numbytes == 0) {
		close(fd);
		DEBUG(ERROR, "%s %d recv :the connection has been closed by peer",pkt.ip, pkt.sockfd);
		return PTR_ERR;
	}
	struct common_header *header = (struct common_header *)newbuf;
	u32 len = ntohl(header->pkg_len);
	DEBUG(INFO, "%s %d %d %u recv %u",pkt.ip, pkt.sockfd, header->pkg_type, len, numbytes);
	if (len > numbytes) {
		newbuf = (char *)malloc(len+1);
		if (newbuf == NULL) {
			close(fd);
			DEBUG(ERROR, "daemon packet_handle: malloc(): %s", strerror(errno));
			return PTR_ERR;
		}
		char *cur_pos =  newbuf;
		memcpy(newbuf, buf, numbytes);
		cur_pos += numbytes;
		len -= numbytes;
		while(len > 0){
			numbytes = recv(fd, cur_pos, len, 0);
			if (numbytes == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK){

					continue;
				}
				close(fd);
				free_buf(newbuf);
				DEBUG(ERROR, "daemon handle_message: recv(): %s", strerror(errno));
				return PTR_ERR;
			} else if (numbytes == 0) {
				close(fd);
				free_buf(newbuf);
				DEBUG(ERROR, "%s %d recv :the connection has been closed by peer",pkt.ip, pkt.sockfd);
				return PTR_ERR;

			}
			DEBUG(INFO, "%s %d %d %u recv %u",pkt.ip, pkt.sockfd, header->pkg_type, len, numbytes);

			cur_pos += numbytes;
			len -= numbytes;
		}
		*(cur_pos) = 0;
	}

	header = (struct common_header *)newbuf;
	len = ntohl(header->pkg_len);

	pkt.len = len;
	pkt.ops_type = header->pkg_type;
	pkt.head = newbuf;
	pkt.end = pkt.tail = (newbuf + len);
	pkt.data = newbuf;

	DEBUG(INFO, "%s %d %d %u NEW_PACKET",pkt.ip, pkt.sockfd, header->pkg_type, len);

	int ret = 0;
	pkt.sockfd = fd;
//	pkt.data = (char*)(newbuf + sizeof( struct pma_pms_header));
//	pkt.len -= sizeof(struct pma_pms_header);
	run_ctl(upload_handle, header->pkg_type , &(pkt));
	close(fd);
	if(newbuf != buf)
	{
		free_buf(newbuf);
	}
	return NULL;
}

int send_ack_to_pms(struct packet* pkt){
	int ack_t =OPS_ACK;
	struct pma_pms_header ack;
	ack.pph_pkg_type = ack_t;
	ack.pph_pkg_version = get_version();
	ack.pph_pkg_dev_type = DEVICETYPE_APP_BM;
	ack.pph_pkg_len = htonl(sizeof(struct pma_pms_header));
	ack.pph_agent_id = htonl(get_pma_id());
	int saddr = get_pma_pms_ip();//get_local_ip();
	ack.src_addr.s_addr = saddr;
	inet_pton(AF_INET,pkt->ip,&ack.dst_addr);

	if (send(pkt->sockfd, (void *)&ack, sizeof(ack), 0) == -1) {
		DEBUG(INFO, "%s %d %d send ack failed %s",pkt->ip, pkt->sockfd, ack_t, strerror(errno));
		return -1;
	}
	return (ack_t == OPS_ACK) ? 0 : 1;
}


