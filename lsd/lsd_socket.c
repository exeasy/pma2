#include <utils/common.h>
#include <utils/utils.h>
#include <lib/thread.h>
#include <lib/event.h>
#include <server/icm.h>
#include "lsd_cmn.h"
#include "lsd_event.h"
#include "lsd_if.h"
#include "lsd_socket.h"


int lsd_h_sock;
int lsd_l_sock;
declare_event_queue(pkt_rcv_queue); 
char recvbuf[MAX_MESSAGE_LEN];
extern struct thread_master* master;
extern struct icm_conf conf;

int create_server(int listen_port, int type){
     if(type !=0&&type!=1){
          printf("Server Type Wrong!\n");
          return -1;
     }
     struct sockaddr_in server_addr;
     int sockfd ;
     if(type == 0) //tcp
          sockfd = socket(AF_INET,SOCK_STREAM,0);
     else
          sockfd = socket(AF_INET,SOCK_DGRAM,0);
     if(sockfd < 0)
     {
          DEBUG(INFO,"can't create socket");
          return -1;
     }
     bzero(&server_addr,sizeof(server_addr));
     server_addr.sin_family = AF_INET;
     server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     server_addr.sin_port = htons(listen_port);
     int serv_len = sizeof(struct sockaddr_in);
     if(bind(sockfd, (struct sockaddr*)&server_addr, serv_len)<0)
     {
          return -1;
      }
     int status = 1;
     setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &status, sizeof (status));
     return sockfd;
}

int lsd_serv_sock(){
	 printf("init server\n");
     if((lsd_h_sock = create_server(IC_H_PORT,1))== -1)
          return -1;
     if((lsd_l_sock = create_server(IC_L_PORT,1))==-1)
          return -1;
	 //add the snmp server
	//if((ic_snmp_sock = create_server(IC_SNMP_PORT,1))==-1)
	//	return -1;
	 //end the snmp server
     thread_add_read(master, lsd_receive,NULL,lsd_h_sock);
     thread_add_read(master, lsd_receive,NULL,lsd_l_sock);
	// thread_add_read(master,snmp_receive,NULL,ic_snmp_sock);
     //printf("init server\n");
     return 0;
}
//receive the message
int lsd_recvmsg(struct in_addr *src, struct in_addr *dst,
		unsigned int *ifindex, struct iovec *message, int sockfd) {
	int retval;
	struct msghdr rmsghdr;
	struct cmsghdr *rcmsgp;
	u_char cmsgbuf[CMSG_SPACE(sizeof (struct in_pktinfo))];
	struct in_pktinfo *pktinfo;
	struct sockaddr_in src_sin;

	rcmsgp = (struct cmsghdr *) cmsgbuf;
	pktinfo = (struct in_pktinfo *) (CMSG_DATA(rcmsgp));
	memset(&src_sin, 0, sizeof(struct sockaddr_in));

	/* receive control msg */
	rcmsgp->cmsg_level = IPPROTO_IP;
	rcmsgp->cmsg_type = IP_PKTINFO;
	rcmsgp->cmsg_len = CMSG_LEN (sizeof (struct in_pktinfo));
	/* rcmsgp = CMSG_NXTHDR (&rmsghdr, rcmsgp); */

	/* receive msg hdr */
	rmsghdr.msg_iov = message;
	rmsghdr.msg_iovlen = iov_count(message);
	rmsghdr.msg_name = (caddr_t) &src_sin;
	rmsghdr.msg_namelen = sizeof(struct sockaddr_in);
	rmsghdr.msg_control = (caddr_t) cmsgbuf;
	rmsghdr.msg_controllen = sizeof(cmsgbuf);

	int status =1;
	//setsockopt(sockfd,IPPROTO_IP,IP_RECVPKTINFO,&status,sizeof(status));
	retval = recvmsg(sockfd, &rmsghdr, 0);

	/*
	 if (retval < 0)
	 zlog_warn ("recvmsg failed: %s", strerror (errno));
	 else if (retval == iov_totallen (message))
	 zlog_warn ("recvmsg read full buffer size: %d", retval);
	 */
	/* source address */
	assert (src);
	memcpy(src, &src_sin.sin_addr, sizeof(struct in_addr));

	/* destination address */
	if (ifindex)
		*ifindex = pktinfo->ipi_ifindex;
	if (dst)
		memcpy(dst, &pktinfo->ipi_addr, sizeof(struct in_addr));

	return retval;

}
//not used
int lsd_sendmsg(struct in_addr *src, struct in_addr *dst,
		unsigned int *ifindex, struct iovec *message) {
	return 0;
}
int iov_count(struct iovec *iov) {
	int i;
	for (i = 0; iov[i].iov_base; i++)
		;
	return i;
}
int iov_totallen(struct iovec *iov) {
	int i;
	int totallen = 0;
	for (i = 0; iov[i].iov_base; i++)
		totallen += iov[i].iov_len;
	return totallen;
}


int lsd_receive (struct thread *thread)
{
	//prepare for next receive procedure.
	int sockfd;
	sockfd = THREAD_FD (thread);
	thread_add_read(master, lsd_receive, NULL, sockfd);
	//read message
	memset (recvbuf, 0, MAX_MESSAGE_LEN);
	struct iovec iovector[2];
	iovector[0].iov_base = recvbuf;
	iovector[0].iov_len = MAX_MESSAGE_LEN;
	iovector[1].iov_base = NULL;
	iovector[1].iov_len = 0;
	struct in_addr src, dst;
	unsigned int ifindex;
	lsd_recvmsg (&src, &dst, &ifindex, iovector, sockfd);

	struct lsd_head *lsd_head;
	lsd_head = (struct lsd_head*)recvbuf;
	//common packet test
	struct autonomous_zone* az = get_autonomous_zone_by_id(lsd_head->area_id);
	if(az == NULL)
	{
		printf("package area id err.");
		return WR_FROMAT_ERR;
	}
	//add by Macro
	int local_if = lsd_head->if_id;
	int nid = lsd_head->r_id;

	struct backbone_eth* eth = get_backbone_by_neighbor_id(az, nid);
	
	if((eth == NULL) || eth->_state  == ETH_DOWN )//ETH_DOWN 
	{
		//printf("Areas missmatch.check neighbor vty configuration.");
		return WR_FROMAT_ERR;
	}
	if(lsd_head->version != LSD_VERSION)
	{
		printf("package format err.");
		return WR_FROMAT_ERR;
	}
	else if(lsd_head->icsum != LSD_CHECKSUM )
	{
		printf("package checksum err.");
		return WR_FROMAT_ERR;
	}
	//dispatch packet.
	struct event_handler_item* item;
	for(item = pkt_rcv_queue.next; item; item = item->next)
		((pkt_rcv_handler)(item->handler))(eth, lsd_head);
	return 0;

}
int lsd_send (struct backbone_eth* eth,struct lsd_head *head)
{
	if( eth->neighbor_pma.rid == 0 || eth->neighbor_pma.pma_ctl_addr.s_addr== 0){//No neighbor
		//DEBUG(INFO,"the interface don't connect any router");
		return -1;
	}
	struct sockaddr_in addr;
	struct in_addr dst_addr;
    dst_addr = eth->neighbor_pma.pma_ctl_addr;
	char ip[20];
	inet_ntop(AF_INET,&dst_addr,ip,20);
	bzero((char*)&addr,   (int)sizeof(addr));
	bcopy(&dst_addr, (char*)&addr.sin_addr,sizeof(dst_addr));
	addr.sin_family = AF_INET;

	head->area_id = eth->az->az_id;
	head->r_id = eth->az->r_id;
	head->if_id = eth->_ifid;
	head->icsum = LSD_CHECKSUM;
	head->version = LSD_VERSION;

	int sock = 0;
	switch(head->pktype)
	{
		case IC_MESSAGE_TYPE_HELLO_H:
		case IC_MESSAGE_TYPE_EXCHANGE:
		case IC_MESSAGE_TYPE_FLOOD:
		case IC_MESSAGE_TYPE_LFA_ADVERTISE:
				sock = lsd_h_sock;
				addr.sin_port = htons(IC_H_PORT);
				break;
		case IC_MESSAGE_TYPE_HELLO_L:
				sock = lsd_l_sock;
				addr.sin_port = htons(IC_L_PORT);
				break;
		default:
				return IO_ERR;
	}
    // set the ttl value 
	int default_ttl = 0;
	int len = sizeof(int);
	getsockopt(sock, IPPROTO_IP, IP_TTL, &default_ttl, &len);
	int ttl = default_ttl;
	if ( conf.outside == 1)
    ttl = 3;
	else if ( conf.outside == 0) 
	ttl = 1;
	
    if(head->pktype == IC_MESSAGE_TYPE_HELLO_H)
        if(setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0 )
        {
            printf("lsd_send error");
            return IO_ERR;
        }

    if(head->pktype == IC_MESSAGE_TYPE_HELLO_L)
        if(setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0 )
        {
            printf("lsd_send error");
            return IO_ERR;
        }
	int ret = sendto(sock, head, head->pklen, 0, (struct sockaddr*)&addr,sizeof(addr));
	setsockopt(sock, IPPROTO_IP, IP_TTL, &default_ttl, sizeof(ttl));

	return 0;

}

