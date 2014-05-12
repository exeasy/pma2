#include <utils/utils.h>
#include <utils/common.h>
#include <server/socket.h>
#include <server/srvconf.h>
#include <lsdb.h>
#include "lsdb_sender.h"


char* create_interface_table_packet(char* iftable,int iflen,int* pkt_len){
	int len = sizeof(Packet_header)+iflen;
	Packet_header* packet = (Packet_header*)malloc(len);
	packet->pkt_type = UP_INTERFACE_INFO;
	packet->pkt_version = PMA_VERSION;
	packet->pkt_len =len;
	memcpy(packet->pkt,iftable,iflen);
	*pkt_len = len;
	return packet;
}


char* create_route_table_packet(char* rt,int rt_len,int* pkt_len){
	int len = sizeof(Packet_header)+rt_len;
	Packet_header* packet = (Packet_header*)malloc(len);
	packet->pkt_type = UP_ROUTE_INFO;
	packet->pkt_version = PMA_VERSION;
	packet->pkt_len = len ;
	memcpy(packet->pkt,rt,rt_len);
	*pkt_len = len;
	return packet;
}

char* create_lsdb_packet(int action,lsdb_Msg* msg,int* len){
	int lsdb_msg_len = sizeof(lsdb_MsgHeader)+ sizeof(lsdb_Msg);
	int lsdb_pkt_len = sizeof(Packet_header) + lsdb_msg_len;
	Packet_header* packet = (Packet_header*)malloc(lsdb_pkt_len);
	packet->pkt_type = action;
	packet->pkt_version = PMA_VERSION;
	packet->pkt_len = lsdb_pkt_len;
	lsdb_MsgHeader* msg_header = (lsdb_MsgHeader*)packet->pkt;
	msg_header->blank= 0x00;
	msg_header->area_id = 0;
	msg_header->version = 1.0;
	msg_header->length = lsdb_msg_len;
	memcpy(msg_header->lsdb_pkt,msg,sizeof(lsdb_Msg));
	*len = lsdb_pkt_len;
	DEBUG(INFO,"create a lsa.......\n");
	return packet;
}
lsdb_Msg* lsdb_msg_create(int area, struct cr_lsdb_link_state* ls){
	lsdb_Msg* msg = (lsdb_Msg*)malloc(sizeof(lsdb_Msg));
	msg->areaid = area;
	msg->rid 	= ls->key.rt_id;
	msg->nrid   = ls->key.n_rt_id;
	msg->ifid   = ls->key.if_id;
	msg->nifid  = ls->key.n_if_id;
	msg->metric = ls->metric;
	msg->seq    = ls->seq;
	msg->status = ls->state;
	msg->age    = ls->age;
	return msg;
}
