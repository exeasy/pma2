#include <utils/common.h>
#include <utils/utils.h>
#include <socket.h>
#include <server.h>
#include <srvconf.h>
#include <control/control.h>
#include <logger/logger.h>
#include <db/dbutils.h>
#include <db/dbm_lsdb.h>
#include "dbm.h"
#include <ddc/ddc.h>
#include <policy/policy_parse.h>
#include <policy/policy_table.h>
#include <timer/timer.h>

//();
extern struct ctl *packet_handler;
struct dbm_conf conf;
int spf_cpt_timer = -1;
int snapshoot_timer = -1;

int dbm_init()
{
	db_open();
	init_empty_lsdb();
	timer_init();
	policy_init();
//	path_compute_start();
//	return 0;
	log_type_init();
	log_init();
	module_init(DBM);
	set_conf_type(DBM_CONF);
	add_ctl(packet_handler,ADDLSA, link_state_handler,1);
	add_ctl(packet_handler,UPDATELSA, link_state_handler,1);
	add_ctl(packet_handler,POLICY_LIST, policy_table_handler,1);
	add_ctl(packet_handler,ACK, ack_handle, 1);
	add_ctl(packet_handler,DBM_CONF, conf_handle, 1);
	add_ctl(packet_handler,OSPF_SPF, spf_signal_handler, 1);
}

int  dbm_start()
{
	module_start();
}

int conf_handle( struct Packet_header *pkt)
{
//	struct dbm_conf conf;
	if ( pkt->pkt_len == 0) return 0;
	memcpy( &conf, pkt->pkt, pkt->pkt_len);
	DEBUG(INFO, "PMAID %d", conf.pma_id );
	DEBUG(INFO, "ROUTERID %d", conf.router_id);
	DEBUG(INFO, "SNAPTIME %d", conf.snapshoot_timeval);
	DEBUG(INFO, "POLICY_TYPE %d", conf.policy_type);
}
int link_state_handler(struct Packet_header *pkt)
{
	if ( pkt->pkt_len == 0) return 0;
	struct lsdb_msgheader * link = (struct lsdb_msgheader*)pkt->pkt;
	struct link_info *lsa = (struct link_info*)link->lsdb_pkt;
	switch ( pkt->pkt_type )
	{
		case ADDLSA:
			insert_link_state(lsa);
#ifndef SPF_DISABLE
			spf_signal_handler(NULL);
#endif
            snapshoot_sender();
			break;
		case UPDATELSA:
			{
				int origin_status = 
					query_link_state_status(lsa->areaid, lsa->rid, lsa->nrid );
				if ( origin_status != lsa->status)
				{
					update_link_state(lsa);
					if ( conf.pma_id == lsa->rid || conf.pma_id == lsa->nrid )
					{
						char* buff = format_lsdb_to_xml(lsa->areaid,conf.pma_id,1);
						module_send_data(buff,strlen(buff), LSDB_INFO);					
                        free(buff);
					}
				}
#ifndef SPF_DISABLE
				if ( origin_status == 3 && lsa->status != 3 )//link broken
				{
					fah_protect_link(conf.router_id , lsa);
				}
				else if ( origin_status == 0 && lsa->status != 0) //link recover
				{
					fah_set_default_timer(conf.router_id, lsa);
				}
#endif
				break;
			}
		default:
			break;
	}
}
int policy_table_handler(struct Packet_header *pkt)
{
	if (pkt->pkt_len == 0 ) return 0;
	u8 * xmldata = pkt->pkt;
	u32 len = pkt->pkt_len;
	process_policy(xmldata, len);
}

//snapshoot timer  function
int send_all_snapshoot(void* args)
{
    int deviceid = conf.pma_id;
 	char* buff = format_all_area_lsdb_to_xml(deviceid);
	module_send_data(buff, strlen(buff), LSDB_INFO); 
    free(buff);
    pthread_detach(pthread_self());
}

int send_policy(int ifid, u32 hello_timer , u32 dead_timer){
	int pkt_len = sizeof(policyMsg) + sizeof(policyMsgHeader);
	Packet_header* pkt = (Packet_header*)malloc(sizeof(Packet_header)+pkt_len);
	pkt->pkt_type = POLICY_INFO;
	pkt->pkt_version = PMA_VERSION;
	pkt->pkt_len = pkt_len + sizeof(Packet_header);
	policyMsgHeader* msgHeader = (policyMsgHeader*)pkt->pkt;
	msgHeader->operation = 0;//Default the timer set//policy->operation;
	msgHeader->pepversion = 3;//Self define version         policy->version;
	policyMsg* msg = (policyMsg*)msgHeader->msg;
	msg->hello_interval =hello_timer;
	msg->dead_interval = dead_timer;
	msg->retransmit_interval  = dead_timer;//miniarrival_interval;
	msg->if_id = ifid;
	send_packet(pkt);
}

int snapshoot_sender()
{
    if ( snapshoot_timer == -1)
    {
        snapshoot_timer = set_timer( conf.snapshoot_timeval , send_all_snapshoot, NULL, 0);
    }
}

int spf_signal_handler(struct Packet_header *pkt)
{
	if ( spf_cpt_timer == -1 )
	{
		spf_cpt_timer = set_timer( SPF_HOLD_TIME , path_compute_start , NULL, 0); 
	}
	else
	{
		update_timer(spf_cpt_timer);
	}
}

int spf_cpt_timer_cancel()
{
	if ( is_computed() && spf_cpt_timer != -1 )
	{
		unset_timer(spf_cpt_timer); 
		spf_cpt_timer = -1;
	}
    pthread_detach(pthread_self());
}
