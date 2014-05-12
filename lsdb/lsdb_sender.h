#ifndef LSDB_SENDER_H

#define LSDB_SENDER_H
#include <server/srvconf.h>
#include <server/socket.h>
typedef struct lsdb_msgheader{
	u8 version;
	u8 blank;
	u16 length;
	u32 area_id;
	u8 lsdb_pkt[1];
}lsdb_MsgHeader;


typedef struct link_state_pkt{
	u32 areaid;
	u32 rid;
	u32 nrid;
	u32 ifid;
	u32 nifid;
	u32 metric;
	u32 status;
	u32 seq;
	struct timeval age;
}lsdb_Msg;

lsdb_Msg* lsdb_msg_create(int area,struct cr_lsdb_link_state* ls);
char* create_lsdb_packet(int action,lsdb_Msg* msg,int* len);
char* create_route_table_packet(char* rt,int rt_len,int* pkt_len);

#endif /* end of include guard: LSDB_SENDER_H */
