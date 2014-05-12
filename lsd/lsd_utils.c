#include <utils/utils.h>
#include <utils/common.h>
#include <utils/list.h>
#include <lsd_if.h>
#include <lsdb/lsdb.h>
#include <lsd_hello.h>
#include <lsd_exchange.h>
#include <lsd_flood.h>
#include <lsd_cmn.h>
#include "lsd_utils.h"

//change lsdb infomation into lsa infomation
void lsdb_to_lsa_info(const struct cr_lsdb_link_state* lsdb, struct link_state_adv* lsa)
{
	lsa->key = lsdb->key;
	lsa->addr = lsdb->addr;
	lsa->metric = lsdb->metric;
	lsa->state = lsdb->state;
	lsa->seq = lsdb->seq;
}

//change lsa infomation into lsdb infomation
void lsa_to_lsdb_info(const struct link_state_adv* lsa, struct cr_lsdb_link_state* lsdb)
{
	lsdb->key = lsa->key;
	lsdb->addr = lsa->addr;
	lsdb->metric = lsa->metric;
	lsdb->state = lsa->state;
	lsdb->seq = lsa->seq;
	gettimeofday(&lsdb->age, NULL);
}

//judge if the lsa is out of date
int is_out_of_date(const struct cr_lsdb_link_state* old)
{
	struct timeval val;
	gettimeofday(&val, NULL);

	unsigned long time =
			(((val.tv_sec - old->age.tv_sec) * 1000000L) + (val.tv_usec - old->age.tv_usec));
	if(time > LSDB_LINK_FRESH_TIME * 15 / 10)
		return TRUE;
	else
		return FALSE;
}

//compare link states' sequence
int link_state_compare(const struct cr_lsdb_link_state* state1, const struct cr_lsdb_link_state* state2)
{
	assert(state1->key.rt_id == state2->key.rt_id);
	assert(state1->key.if_id == state2->key.if_id);
	assert(state1->key.n_rt_id == state2->key.n_rt_id);
	assert(state1->key.n_if_id == state2->key.n_if_id);

	if(state1->seq > state2->seq)
		return 1;
	else if (state1->seq == state2->seq)
		return 0;
	else
		return -1;
}

//compare lsas' sequence
int lsa_compare(const struct link_state_adv* lsa1, const struct link_state_adv* lsa2)
{
	assert(lsa1->key.rt_id == lsa2->key.rt_id);
	assert(lsa1->key.if_id == lsa2->key.if_id);
	assert(lsa1->key.n_rt_id == lsa2->key.n_rt_id);
	assert(lsa1->key.n_if_id == lsa2->key.n_if_id);

	if(lsa1->seq > lsa2->seq)
		return 1;
	else if (lsa1->seq == lsa2->seq)
		return 0;
	else
		return -1;
}

//judge if the link states' are equal
int is_keys_equals(const struct link_state_key* key1, const struct link_state_key* key2)
{
	if((key1->n_rt_id == key2->n_rt_id)
			&&
		(key1->n_if_id == key2->n_if_id)
			&&
		(key1->rt_id == key2->rt_id)
			&&
		(key1->if_id == key2->if_id))
	{
		return TRUE;
	}
	return FALSE;
}

//check the lsa from the local lsdb
int check_local_lsdb(struct backbone_eth* bb_link, struct cr_lsdb_link_state* state)
{
	if(state->key.rt_id != get_backbone_eth_area(bb_link)->r_id)
		return FALSE;

	id_t lsdb = lsdb_get_eth_handle(bb_link);
	struct cr_lsdb_link_state local_state;
	if(cr_lsdb_link_state_find(&state->key, &local_state, lsdb) == NO_ERR)
	{
		if(link_state_compare(&local_state, state) != 0)
		{
			DEBUG(INFO,"<LSA>\t\t%d:%d--%d:%d seq(%d)\tFLOOD BACK(NOT EQUAL LOCAL SEQ %d)",
				state->key.rt_id, state->key.if_id, state->key.n_rt_id, state->key.n_if_id, state->seq,
				local_state.seq);
#ifndef __EXCHANGE_DEBUG__
			struct link_state_adv ic_lsa;
			lsdb_to_lsa_info(&local_state, &ic_lsa);
			flood_add_flood_lsa(&ic_lsa, bb_link);
#endif
		}else{
			DEBUG(INFO,"<LSA>\t\t%d:%d--%d:%d seq(%d)\tDROP(EQUAL LOCAL SEQ %d)",
				state->key.rt_id, state->key.if_id, state->key.n_rt_id, state->key.n_if_id, state->seq,
				local_state.seq);
		}
	}
	return TRUE;
}

//store the lsa into the lsdb
void accept_lsa_into_lsdb(struct backbone_eth* bb_link, struct link_state_adv *ic_lsa)
{
	struct cr_lsdb_link_state lsa;
	lsa_to_lsdb_info(ic_lsa, &lsa);
	if(check_local_lsdb(bb_link, &lsa) == TRUE)
		return;

	struct cr_lsdb_link_state link;
	id_t lsdb = lsdb_get_eth_handle(bb_link);
	int err_code = cr_lsdb_link_state_find(&lsa.key, &link, lsdb);
	if(err_code == NOT_FOUND_ERR)
	{
		cr_lsdb_link_state_add(&lsa, lsdb);
		DEBUG(INFO,"<LSA>\t\t%d:%d--%d:%d seq(%d)\tADD (ITEM NOT EXISTS)", lsa.key.rt_id, lsa.key.if_id,
				lsa.key.n_rt_id, lsa.key.n_if_id, lsa.seq);
#ifndef __EXCHANGE_DEBUG__
		flood_lsa_forward(ic_lsa, bb_link);
#endif
	}else if(err_code == NO_ERR)
	{

		if(is_out_of_date(&link))
		{

			cr_lsdb_link_state_update(&lsa, lsdb);
			DEBUG(INFO,"<LSA>\t\t%d:%d--%d:%d seq(%d)\tSTATE ACCEPT (LOCAL STATE OVERTIME)",
				lsa.key.rt_id, lsa.key.if_id, lsa.key.n_rt_id, lsa.key.n_if_id, lsa.seq);
#ifndef __EXCHANGE_DEBUG__
			flood_lsa_forward(ic_lsa, bb_link);
#endif
			return;
		}
		int ra = link_state_compare(&link, &lsa);

		if(ra < 0)
		{
			cr_lsdb_link_state_update(&lsa, lsdb);
			DEBUG(INFO,"<LSA>\t\t%d:%d--%d:%d seq(%d)\tSTATE ACCEPT (NEWER SEQ RECEIVED)",
				lsa.key.rt_id, lsa.key.if_id, lsa.key.n_rt_id, lsa.key.n_if_id, lsa.seq);
#ifndef __EXCHANGE_DEBUG__
			flood_lsa_forward(ic_lsa, bb_link);
#endif
		}else if(ra > 0){
			DEBUG(INFO,"<LSA>\t\t%d:%d--%d:%d seq(%d)\tFLOOD BACK(OLDER SEQ RECEIVED)",
				lsa.key.rt_id, lsa.key.if_id,lsa.key.n_rt_id, lsa.key.n_if_id, lsa.seq);
#ifndef __EXCHANGE_DEBUG__
			lsdb_to_lsa_info(&link, ic_lsa);
			flood_add_flood_lsa(ic_lsa, bb_link);
#endif
		}else{
			DEBUG(INFO,"<LSA>\t\t%d:%d--%d:%d seq(%d)\tDROP(SAME SEQ RECEIVED)",
				lsa.key.rt_id, lsa.key.if_id, lsa.key.n_rt_id, lsa.key.n_if_id, lsa.seq);
		}
	}
}


