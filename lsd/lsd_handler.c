#include <utils/utils.h>
#include <utils/common.h>
#include "lsd_if.h"
#include "lsd_hello.h"
#include "router/interface.h"
#include <lsdb/lsdb.h>
#include "lsd_handler.h"

extern struct lsd_router g_lsd_router;


void hello_changed_handler(struct backbone_eth* eth,
		enum priority_type type, enum lsd_status status){
		struct lsd_neighbor_info info;
		state_changer state_info;
		hello_get_neighbor_info(eth, &info);

		id_t lsdb = lsdb_get_eth_handle(eth);
		struct cr_lsdb_link_state state;
		struct cr_lsdb_link_state  h_result;
		struct cr_lsdb_link_state  l_result;

		if(type == HIGH_PRIORITY_DETECTION)
		{
			DEBUG(INFO,"<HIGH>%d:%d--%d:%d status:%d\n", eth->az->r_id, eth->_ifid,
				info.rt_id, info.if_id, status);

			state.key.rt_id = eth->az->r_id;
			state.key.if_id = eth->_ifid;
			state.key.n_rt_id = info.rt_id;
			state.key.n_if_id = info.if_id;
			state.metric = get_metric_of_backbone(eth);
			get_address_of_backbone(eth, &state.addr);

			//****************************************
			char interf_addr[256] = {0};
			inet_ntop(AF_INET, &state.addr, interf_addr, 256);
			DEBUG(INFO, "[dispatch]Interface:%d->%d, metric is : %d address is : %s\n", state.key.rt_id, state.key.if_id, state.metric, interf_addr);
			//***************************************

			gettimeofday(&state.age, NULL);

			if (NOT_FOUND_ERR == cr_lsdb_link_state_find(&state.key, &h_result, lsdb))
			{
				state.seq = 0;
				if (status == 1)//connect
				{
					state.state = 0x02;
				}
				else //disconnect
				{
					state.state = 0x00;
				}
				assert(cr_lsdb_link_state_add(&state, lsdb) == NO_ERR);
				if (status == 1)
				{
    //                #ifdef OSPF_VERSION
	//				struct link_state_key* key = (struct link_state_key*)malloc(sizeof(struct link_state_key));
	//				memcpy(key,&state.key,sizeof(state.key));
	//				RUN_EVENT(LINK_UP,key);
					exchange_send_start(eth);
					flood_start(eth);
					flood_spread_backbone_link_state(eth, state.state);
    //                #endif

				}
				else
				{
    //                #ifdef OSPF_VERSION
	//				struct link_state_key* key = (struct link_state_key*)malloc(sizeof(struct link_state_key));
	//				memcpy(key,&state.key,sizeof(state.key));
	//				RUN_EVENT(LINK_DOWN,key);
					flood_spread_backbone_link_state(eth, state.state);
					exchange_terminate(eth);
					flood_terminate(eth);
    //                #endif
				}

			}
			else
			{
				state.seq = h_result.seq++;
				state.state = h_result.state;

				if (status == 1)
				{
					state_info.data = state.state;
					state_info.field.hello_high = 1;
					state.state = state_info.data;
				}
				else
				{
					state_info.data = state.state;
					state_info.field.hello_high = 0;
					state.state = state_info.data;
				}
				assert(cr_lsdb_link_state_update(&state, lsdb) == NO_ERR);

				if (status == 1)
				{
    //                #ifdef OSPF_VERSION
	//				struct link_state_key* key = (struct link_state_key*)malloc(sizeof(struct link_state_key));
	//				memcpy(key,&state.key,sizeof(state.key));
	//				RUN_EVENT(LINK_UP,key);
	//				//fah_set_default_timer(eth);
					exchange_send_start(eth);
					flood_start(eth);
					flood_spread_backbone_link_state(eth, state.state);
    //                #endif
				}
				else
				{
    //                #ifdef OSPF_VERSION
	//				struct link_state_key* key = (struct link_state_key*)malloc(sizeof(struct link_state_key));
	//				memcpy(key,&state.key,sizeof(state.key));
	//				RUN_EVENT(LINK_DOWN,key);
					flood_spread_backbone_link_state(eth, state.state);
					exchange_terminate(eth);
					flood_terminate(eth);
    //                #endif
					//cpi_policy_get(eth);
	#ifdef ONIXCLIENT
					//xml_event(lsdb);
	#endif
				}
			}

		}

		else
		{
			DEBUG(INFO, "<LOW>%d:%d--%d:%d status:%d\n", eth->az->r_id, eth->_ifid,
				info.rt_id, info.if_id, status);
			state.key.rt_id = eth->az->r_id;
			state.key.if_id = eth->_ifid;
			state.key.n_rt_id = info.rt_id;
			state.key.n_if_id = info.if_id;
			state.metric = get_metric_of_backbone(eth);
			get_address_of_backbone(eth, &state.addr);

			gettimeofday(&state.age, NULL);

			if (NOT_FOUND_ERR == cr_lsdb_link_state_find(&state.key, &l_result, lsdb))
			{
				assert(cr_lsdb_link_state_add(&state, lsdb) == NO_ERR);

				state.seq = 0;
				if (status == 1)
				{
					state.state = 0x01;
                  //  #ifdef OSPF_VERSION
					flood_spread_backbone_link_state(eth, state.state);
                   // #endif OSPF_VERSION
				}
				else
				{
					state.state = 0x00;
                //    #ifdef OSPF_VERSION
					flood_spread_backbone_link_state(eth, state.state);
                 //   #endif
				}
			}
			else
			{
				state.seq = l_result.seq++;
				state.state = l_result.state;
				if (status == 1)
				{
					state_info.data = state.state;
					state_info.field.hello_low = 1;
					state.state = state_info.data;
                    // #ifdef OSPF_VERSION
					flood_spread_backbone_link_state(eth, state.state);
                    // #endif
				}
				else
				{
					state_info.data = state.state;
					state_info.field.hello_low = 0;
					state.state = state_info.data;
                    // #ifdef OSPF_VERSION
					flood_spread_backbone_link_state(eth, state.state);
                     //#endif
				}

				assert(cr_lsdb_link_state_update(&state, lsdb) == NO_ERR);
			}
		}

}

void ace_up_handler(struct access_eth* eth)
{
     struct autonomous_zone * paz = g_lsd_router.azs;
     id_t * lsdb;
     struct cr_lsdb_link_state state;

   struct in_addr result;
   char str[100];
    get_address_of_access(eth, &result);
    inet_ntop(AF_INET, &result, str,24);
   DEBUG(INFO,"Access interface ID:%d Address :%s\n", eth->_ifid, str);
   while (paz != NULL)
   {
           lsdb = &paz->lsdb;
           state.key.rt_id = paz->r_id;
           state.key.if_id = eth->_ifid;
           state.key.n_rt_id = 0;
           state.key.n_if_id = 0;
           state.addr = result;
           state.metric = get_metric_of_access(eth);
           state.seq = 0;
           state.state = 0x00;
           gettimeofday(&state.age, NULL);
           struct link_state_key* key = &state.key;
           struct cr_lsdb_link_state link;
           if(cr_lsdb_link_state_find(key,&link,*lsdb)==NOT_FOUND_ERR)
                   assert(cr_lsdb_link_state_add(&state, *lsdb) == NO_ERR);
           else
                   cr_lsdb_link_state_update(&state,*lsdb);
           paz = paz->next;
   }
}

/*
void hello_link_handler(struct ic_backbone_eth* eth,
		enum priority_type type, enum lsd_status status)
{
    if(type != HIGH_PRIORITY_DETECTION) return;
	struct lsd_neighbor_info info;
		state_changer state_info;
		ic_hello_get_neighbor_info(eth, &info);

    int bgpid = htonl(self_id);
    char* if_xml = (char*)malloc(1024);
	time_t raw_time;
	struct tm* time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);
	int pos = 0;
	sprintf(if_xml+pos,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");pos = strlen(if_xml);
	sprintf(if_xml+pos,"<INTERFACE_INFO>\n");pos = strlen(if_xml);
	sprintf(if_xml+pos,"<timestamp>%4d-%d-%d %d:%d:%d</timestamp>\n",
		1900+time_info->tm_year,
		1+time_info->tm_mon,
		time_info->tm_mday,
		time_info->tm_hour,
		time_info->tm_min,
		time_info->tm_sec);pos = strlen(if_xml);
	sprintf(if_xml+pos,"<PMA id=\"%d\">\n",bgpid);pos = strlen(if_xml);
	struct interface* ifp;
	struct listnode* temp;
	LIST_LOOP(iflist,ifp,temp)
	{
		if(ifp->info != NULL)//this interface is we want
		{
			struct ic_backbone_eth* bbe = (struct ic_backbone_eth*)(ifp->info);
			if(bbe->neighbour == NULL)
				continue;
            char bgp_id[24];
            inet_ntop(AF_INET,&bgpid,bgp_id,24);
			sprintf(if_xml+pos,"<link>\n");pos = strlen(if_xml);
			sprintf(if_xml+pos,"<router_id>%s</router_id>\n",bgp_id);pos = strlen(if_xml);
			char neighbor[24];
			inet_ntop(AF_INET,&bbe->neighbour->id,neighbor,24);
			sprintf(if_xml+pos,"<neighbor_id>%s</neighbor_id>\n",neighbor);pos = strlen(if_xml);
			struct hello_master* hello = (struct hello_master*)bbe->hello_master;

			int status;
			if(hello == NULL)
			    status = 0;
			else 
				status = hello->high_status;
			
			sprintf(if_xml+pos,"<link_state>%d</link_state>\n",status);pos = strlen(if_xml);
			sprintf(if_xml+pos,"</link>\n");pos = strlen(if_xml);
		}
	}
	sprintf(if_xml+pos,"</PMA>\n");pos = strlen(if_xml);
	sprintf(if_xml+pos,"</INTERFACE_INFO>\n");pos = strlen(if_xml);
	printf("%s\n",if_xml);
	memcpy(iftable_xml,if_xml,pos);
    iftable_len = pos;
	char* packet = create_interface_table_packet(if_xml,pos,&pos);
	send_packet(packet);


/*
	char local_id[24];
	char neighbor_id[24];
	time_t raw_time;
	struct tm* time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);
	//char local_if[24];
	struct interface* intface = eth->interface;
	inet_ntop(AF_INET,&eth->az->r_id,local_id,24);
	inet_ntop(AF_INET,&eth->neighbour->id,neighbor_id,24);
	char* pkt = (char*)malloc(1024);
	int pos = 0;
	sprintf(pkt+pos,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");pos = strlen(pkt);
	sprintf(pkt+pos,"<INTERFACE_INFO>\n");pos = strlen(pkt);
	sprintf(pkt+pos,"<timestamp>%4d-%d-%d %d:%d:%d</timestamp>\n",
		1900+time_info->tm_year,
		1+time_info->tm_mon,
		time_info->tm_mday,
		time_info->tm_hour,
		time_info->tm_min,
		time_info->tm_sec);pos = strlen(pkt);
	sprintf(pkt+pos,"<PMA id=\"%d\">\n",self_id);pos = strlen(pkt);
	sprintf(pkt+pos,"<link>\n");pos = strlen(pkt);
	sprintf(pkt+pos,"<router_id>%s</router_id>\n",local_id);pos = strlen(pkt);
	sprintf(pkt+pos,"<neighbor_id>%s</neighbor_id>\n",neighbor_id);pos = strlen(pkt);	
	if(type == HIGH_PRIORITY_DETECTION)
	{
		dispatch_printf("<HIGH>%d:%d--%d:%d status:%d\n", eth->az->r_id, eth->interface_id,
				info.rt_id, info.if_id, status);
		sprintf(pkt+pos,"<link_state>%d</link_state>\n",status);pos = strlen(pkt);
	}
	sprintf(pkt+pos,"</link></PMA></INTERFACE_INFO>\n");pos = strlen(pkt);
	int len =0;
    memcpy(iftable_xml,pkt,pos);
    iftable_len = pos;
	char* packet = create_interface_table_packet(pkt,pos,&len);
	send_packet(packet);
	
		/*

		if (status == 1)//connect
		{
			//state.state = 0x02;
			
		}
		else //disconnect
		{
			//state.state = 0x00;
		}
		
	}
	else
	{
		dispatch_printf("<LOW>%d:%d--%d:%d status:%d\n", eth->az->r_id, eth->interface_id,
				info.rt_id, info.if_id, status);
		if (status == 1)//connect
		{
			//state.state = 0x02;
		}
		else //disconnect
		{
			//state.state = 0x00;
		}
	}
}*/
