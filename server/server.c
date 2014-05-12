#include <utils/common.h>
#include <utils/utils.h>
#include <lib/event.h>
#include <control/control.h>
#include <srvconf.h>
#include <socket.h>
#include <server.h>

define_event_queue_list(status_transfer, M_STATUS_TOTAL)

char* module[] = {"ICM", "DBM", "PEM", "BM", "PMS"};
#define SELF	module[self_module-1]
u_char self_module;
int module_status;
struct ctl *packet_handler;
int conf_type;

int module_start()
{
    return module_set_status(M_STATUS_INITED);
}

int set_conf_type(int type)
{
	conf_type = type;
}
void test(void* args)
{
	printf("Module Status Trace Test\n");
}

int module_init(int moduleid)
{
	self_module = moduleid;
    add_event_handler(&status_transfer[M_STATUS_INITED], module_register);
    add_event_handler(&status_transfer[M_STATUS_LOGGED], module_daemon);
   // add_event_handler(&status_transfer[M_STATUS_READY], module_daemon);
    add_event_handler(&status_transfer[M_STATUS_OFFLINE], module_start);

	DEBUG(INFO,"%d %s",self_module, SELF);
	init_ctls();
	packet_handler = create_ctl(1024);
	return 0;
}


int module_register()
{
	module_status = M_STATUS_LOGGING ;
	int ret = -1;
	ret = srv_connect();
	if( ret ) 
	{
		close(ret);
		sleep(5);
		module_set_status(M_STATUS_INITED);
	}


	int reg_module = self_module;
	int packet_len = sizeof( Packet_header ) + sizeof(int);
	Packet_headerp reg_packet = (Packet_headerp)malloc(packet_len);
	reg_packet->pkt_type = MODREG;
	reg_packet->pkt_version = PMA_VERSION;
	reg_packet->pkt_len = sizeof(Packet_header) + sizeof(int);
	reg_packet->checksum = 0x123456;
	memcpy(reg_packet->pkt, &reg_module, sizeof(int));
	send_packet(reg_packet);
	sleep(1);
	module_get_data();
}

int module_daemon()
{
	module_status = M_STATUS_WAITING;
	fd_set scanfd;
	int bm_sock = get_bm_sock();
	int err = -1;
	for(;;){
		if ( con_status == DISCONNECT )
			break;
		struct timeval timeout;
		timeout.tv_sec =5;
		timeout.tv_usec = 0;
		int maxfd = bm_sock + 1;
		FD_ZERO(&scanfd);
		FD_SET(bm_sock,&scanfd);
		err = select(maxfd,&scanfd,NULL,NULL,&timeout);
		switch(err){
			case 0:break;
			case -1:break;
			default:
					{
						int ret = module_get_data();
						if(con_status==DISCONNECT && ret == -1)
						{
							DEBUG(ERROR,"Socket lost connection.Reconnecting...");
							close(bm_sock);
						}
						break;
					}
		}
	}
	if( con_status == DISCONNECT )
	{
		close(bm_sock);
		return module_set_status( M_STATUS_OFFLINE );
	}
}

int module_send_data(char *data, int length, int type)
{
	if(length<=0){
		DEBUG(ERROR,"send data's length is wrong");
		return -1;
	}
	char* data_content = data;
	int packet_len = sizeof(Packet_header)+length;
	Packet_headerp packet = (Packet_headerp)malloc(packet_len);
	packet->pkt_type= type;
	packet->pkt_len = packet_len;
	packet->pkt_version = PMA_VERSION;
	memcpy((char*)packet+sizeof(Packet_header),data_content,length);

	return send_packet(packet);
}


int module_get_data()
{
	struct Packet_header* pkt;
	pkt = recv_packet();
	if(pkt == NULL){return -1;}
	int pkt_type = pkt->pkt_type;
	int pkt_version = pkt->pkt_version;
	int pkt_len = pkt->pkt_len - sizeof(struct Packet_header);
    pkt->pkt_len = pkt_len;
	if(pkt_version != PMA_VERSION)
	{
		DEBUG(INFO,"wrong version packet received,throw it away");
		return 0;
	}
	DEBUG(INFO,"Data received:Type[%d]Length[%d]",pkt_type,pkt_len);
	int ctl_flag = 0;
	if( module_status == M_STATUS_WORKING )// Normal Packet
		ctl_flag = 1;
	if( module_status == M_STATUS_LOGGING && pkt->pkt_type == ACK )//LOGGING ACK
		ctl_flag = 1;
	if( pkt->pkt_type == conf_type )//CONF PACKET
		ctl_flag = 1;
	if( ctl_flag )
	run_ctl(packet_handler, pkt->pkt_type, pkt);
	if( pkt->pkt_type == conf_type )
		module_status =  M_STATUS_WORKING ;
	if (pkt )
		free(pkt);
}

int ack_handle(struct Packet_header* pkt)
{
	struct response_message * msg = (struct response_message*)pkt->pkt;
	if(msg->des_mod != self_module)
	{
		return -1;
	}
	if(msg->ret != 0 )
	{
		return -1;
	}
	DEBUG(INFO,"LOGIN ACK received, [%d]:[%s]\n",\
			msg->des_mod, msg->info);
    return module_set_status(M_STATUS_LOGGED);
}
int module_set_status(int status)
{
    module_status = status;
    traverse_event(&status_transfer[module_status]);
}
