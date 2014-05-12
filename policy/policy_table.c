#include "utils/common.h"
#include "utils/utils.h"
#include "policy_table.h"
struct _policy_table *g_pt;

int policy_init()
{
	int len = 0;
	len = sizeof(struct _policy_table)+sizeof(struct _policy_element)*POLICY_TABLE_LEN;
	if ( g_pt == NULL )
		g_pt = (struct _policy_table*)malloc(len);
	memset( g_pt, 0x00, len );
	init_policy_table( g_pt );
	DEBUG( INFO, "INIT POLICY TABLE[%s][%d]\n", __FILE__, __LINE__ );
	return 0;
}

	int
init_policy_table( struct _policy_table *p )
{
	if( p == NULL )
	{
		DEBUG( ERROR, "[%s][%d]policy NULL error\n", __FILE__, __LINE__ );
		return -1;
	}

	p->policy_table_flag = POLICY_TABLE_USED;
	p->policy_table_len = POLICY_TABLE_LEN;
	p->used_len = 0;
	p->current_used_policy = 0;

	DEBUG( INFO, "[%s][%d]init_policy_table function \n", __FILE__, __LINE__ );

	return 0;
}



int add_policy( struct _policy_table *policy_table, struct _policy_element *p_elem )
{
	int lower = 0;
	int up = 0;

	if( p_elem == NULL || policy_table == NULL )
	{
		DEBUG( ERROR, "input param is null" );
		return -1;
	}
	insert_policy_table( policy_table, p_elem );
	return 0;
}

	int
is_policy_init( struct _policy_table *p )
{
	int ret = -1;

	if( p == NULL )
	{
		DEBUG( ERROR, "[%s][%d]policy NULL error\n", __FILE__, __LINE__ );
		return -1;
	}

	if( (p->policy_table_flag&POLICY_TABLE_USED) == POLICY_TABLE_USED )
	{
		ret = 0;
	}

	return ret;
}

	int
insert_policy_table( struct _policy_table *policy, struct _policy_element *elem )
{
	if( policy == NULL )
	{
		DEBUG( ERROR, "[%s][%d]policy NULL error\n", __FILE__, __LINE__ );
		return -1;
	}

	if( is_policy_init(policy) != 0 )
	{	
		DEBUG( ERROR, "[%s][%d]policy does not init\n", __FILE__, __LINE__ );
		return -1;
	}

	if( policy->used_len >= policy->policy_table_len )
	{
		DEBUG( ERROR, "[%s][%d]policy's num is greater than default POLICY NUM\n", __FILE__, __LINE__ );
		return -1;
	}

	struct _policy_element *p_elem = NULL;
	p_elem= &(policy->p_element[policy->used_len]);
	if( p_elem == NULL )
	{
		DEBUG( ERROR, "[%s][%d]p_elem is NULL error\n", __FILE__, __LINE__ );
		return -1;
	}

	p_elem->operation = elem->operation;
	p_elem->up_limit = elem->up_limit;
	p_elem->lower_limit = elem->lower_limit;
	p_elem->value[0] = elem->value[0];
	p_elem->value[1] = elem->value[1];

	policy->used_len++;

	return 0;
}

	int
print_policy_table( struct _policy_table *p )
{
	int i = 0;

	if( NULL == p )
	{
		DEBUG( ERROR, "[%s][%d]input error\n", __FILE__, __LINE__ );
		return -1;
	}

	DEBUG( INFO, "[%s][%d]policy_table_flag=[%d], policy_table_len=[%d]\n", __FILE__, __LINE__, p->policy_table_flag, p->policy_table_len );
	DEBUG( INFO, "[%s][%d]used_len=[%d]\n", __FILE__, __LINE__, p->used_len );

	for( i=0; i<p->used_len; i++ )
	{
		DEBUG( INFO, "[%s][%d]lower=[%u], up=[%u]\n", __FILE__, __LINE__, (p->p_element[i]).lower_limit, (p->p_element[i]).up_limit );
		DEBUG( INFO, "[%s][%d]value1=[%lld], value2=[%lld]\n", __FILE__, __LINE__, (p->p_element[i]).value[0], (p->p_element[i]).value[1] );
	}
}

	int
delete_policy_table( struct _policy_table * p )
{
	if( is_policy_init(p) != 0 )
	{	
		DEBUG( ERROR, "[%s][%d]policy does not init\n", __FILE__, __LINE__ );
		return -1;
	}
}

	int
find_policy_table( struct _policy_table *p, int key )
{
	int i = 0;

	if( NULL == p )
	{
		DEBUG( ERROR, "[%s][%d]input error\n", __FILE__, __LINE__ );
		return -1;
	}

	if( is_policy_init(p) != 0 )
	{	
		DEBUG( ERROR, "[%s][%d]policy does not init\n", __FILE__, __LINE__ );
		return -1;
	}

	for( i=0; i<p->used_len; i++ )
	{
		if( key >= (p->p_element[i]).lower_limit && key < (p->p_element[i]).up_limit )
		{
			DEBUG( INFO, "value1=[%lld], value2=[%lld]\n", (p->p_element[i]).value[0], (p->p_element[i]).value[1] );
			return i;//add by macro
		}
	}
	return 0;
}

