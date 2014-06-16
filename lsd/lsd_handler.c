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
			DEBUG(INFO,"<HIGH>%d:%d--%d:%d status:%d\n", eth->az->device_id, eth->_ifid,
				info.rt_id, info.if_id, status);

			state.key.rt_id = eth->az->device_id;
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
#ifndef EXCHANGE_DISABLE
					exchange_send_start(eth);
#endif
#ifndef FLOOD_DISABLE
					flood_start(eth);
					flood_spread_backbone_link_state(eth, state.state);
#endif
    //                #endif

				}
				else
				{
    //                #ifdef OSPF_VERSION
	//				struct link_state_key* key = (struct link_state_key*)malloc(sizeof(struct link_state_key));
	//				memcpy(key,&state.key,sizeof(state.key));
	//				RUN_EVENT(LINK_DOWN,key);
#ifndef EXCHANGE_DISABLE
					exchange_terminate(eth);
#endif
#ifndef FLOOD_DISABLE
					flood_spread_backbone_link_state(eth, state.state);
					flood_terminate(eth);
#endif
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
#ifndef EXCHANGE_DISABLE
					exchange_send_start(eth);
#endif
#ifndef FLOOD_DISABLE
					flood_start(eth);
					flood_spread_backbone_link_state(eth, state.state);
#endif
    //                #endif
				}
				else
				{
    //                #ifdef OSPF_VERSION
	//				struct link_state_key* key = (struct link_state_key*)malloc(sizeof(struct link_state_key));
	//				memcpy(key,&state.key,sizeof(state.key));
	//				RUN_EVENT(LINK_DOWN,key);
#ifndef EXCHANGE_DISABLE
					exchange_terminate(eth);
#endif
#ifndef FLOOD_DISABLE
					flood_spread_backbone_link_state(eth, state.state);
					flood_terminate(eth);
#endif
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
			DEBUG(INFO, "<LOW>%d:%d--%d:%d status:%d\n", eth->az->device_id, eth->_ifid,
				info.rt_id, info.if_id, status);
			state.key.rt_id = eth->az->device_id;
			state.key.if_id = eth->_ifid;
			state.key.n_rt_id = info.rt_id;
			state.key.n_if_id = info.if_id;
			state.metric = get_metric_of_backbone(eth);
			get_address_of_backbone(eth, &state.addr);

			gettimeofday(&state.age, NULL);

			if (NOT_FOUND_ERR == cr_lsdb_link_state_find(&state.key, &l_result, lsdb))
			{
				state.seq = 0;
				if (status == 1)
				{
					state.state = 0x01;
				}
				else
				{
					state.state = 0x00;
				}
				assert(cr_lsdb_link_state_add(&state, lsdb) == NO_ERR);
				if (status == 1)
				{
//					state.state = 0x01;
                  //  #ifdef OSPF_VERSION
#ifndef FLOOD_DISABLE
					flood_spread_backbone_link_state(eth, state.state);
                   // #endif OSPF_VERSION
#endif
				}
				else
				{
//					state.state = 0x00;
                //    #ifdef OSPF_VERSION
#ifndef FLOOD_DISABLE
					flood_spread_backbone_link_state(eth, state.state);
                 //   #endif
#endif
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
#ifndef FLOOD_DISABLE
					flood_spread_backbone_link_state(eth, state.state);
#endif
                    // #endif
				}
				else
				{
					state_info.data = state.state;
					state_info.field.hello_low = 0;
					state.state = state_info.data;
                    // #ifdef OSPF_VERSION
#ifndef FLOOD_DISABLE
					flood_spread_backbone_link_state(eth, state.state);
#endif
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
           state.key.rt_id = paz->device_id;
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

