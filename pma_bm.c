#include <utils/common.h>
#include <utils/utils.h>
#include <comm/handler.h>
#include <comm/header.h>
#include <comm/comm.h>
#include <conf/conf.h>
#include <reg/reg.h>
#include <control/control.h>
#include <vty/vty.h>
#include <snmp/includes/snmpcore.h>
#include <logger/logger.h>
int test(void *args)
{
	printf("This is a test pkt handle\n");
}
extern struct conf pma_conf;
int main()
{
	
/*
	init_ctls();
	struct ctl* pms_pkt_ctl = create_ctl(1000);
	add_ctl(pms_pkt_ctl, OPS_PMS_POLICY_SEND, test);
	add_ctl(pms_pkt_ctl, OPS_PMS_MARI_SEND, test);
	run_ctl(pms_pkt_ctl, 1, NULL);
	run_ctl(pms_pkt_ctl, OPS_PMS_MARI_SEND, NULL);
	return 0;
*/
	init_ctls();
	printf("This is basemodule\n");
	DEBUG(INFO,"BaseModule Started");
	struct timeval detail_time;
	gettimeofday(&detail_time, NULL);
	pma_start_time = detail_time.tv_sec;
	
	int ret = 0;
	DEBUG(INFO,"INIT CONFIGURE MODULE");
	ret = USING(conf);
	if (ret != 0) {
		DEBUG(ERROR,"INIT CONFIGURE MODULE FAILED");
		return -1;
	}

	
	DEBUG(INFO,"INIT COMMUNICATION MODULE");
	ret = USING(comm);
	if (ret != 0) {
		DEBUG(ERROR,"INIT INFORMATION COLLECTION MODULE");
		return -1;
	}

	ret = fork_daemon();
	if(ret == 1)//Logger Server
	{
	  log_reset();
	  log_daemon();
	  return 0;
	}
	else//Logger Client
	{
	//Wait the server to start completily
	sleep(2);
	log_init();
	log_type_init();	
	}
#ifndef LOCAL_TEST
	DEBUG(INFO,"LOGIN...");
	do{
		logger("PMA_REGISTER","agent register packet");
		ret = agent_reg();
		if( ret != 0) 
		{
			DEBUG(ERROR,"LOGIN ERROR");
		}
		else
		{
			logger("PMA_REGISTER_OK","agent register ok packet");
			DEBUG(INFO,"LOGIN SUCCESS");
			break;
		}
		sleep(10);
	}
	while(ret != 0);

	char routerip[24];
	char routerid[24];
	inet_ntop(AF_INET, &pma_conf.ic_config.router_ip, routerip,24);
	snmp_init(routerip);
	if( get_protocol_type() == OSPF_ROUTER )
	{
		get_ospf_routerid(routerid);
	}
	else if ( get_protocol_type() == BGP_ROUTER )
	{
		get_bgp_routerid(routerid);
	}
	inet_pton(AF_INET, routerid, &pma_conf.dbm_config.router_id);
	DEBUG(INFO,"AGENT INIT....");
	do{
		ret = agent_init_request(routerid);
		if( ret != 0)
		{
			DEBUG(ERROR, "AGENT INIT ERROR");
		}
		else{
			DEBUG(INFO,"AGENT INITED");
			break;
		}
		sleep(10);
	}
	while( ret != 0);

#endif
    ret = USING(timer);
    if (ret != 0 ){
        DEBUG(ERROR, "INIT TIMER FAILED");
        return -1;
    }

	/* start the heart beat */
	set_timer( 5, heart_beat_send, NULL, 0);

	ret = USING(vty);
	if (ret != 0) {
		DEBUG(ERROR, "INIT VTY FAILED");
		return -1;
	}

	ret = USING(interior_daemon);
	if (ret != 0) {
		DEBUG(ERROR, "INIT LOCAL_DAEMON FAILED");
		return -1;
	}

}
