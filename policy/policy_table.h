#ifndef POLICY_TABLE_H
#define POLICY_TABLE_H

#define POLICY_TABLE_LEN 20
#define POLICY_HASH_TABLE_LEN 200

#define POLICY_TABLE_USED 0x00
#define POLICY_TABLE_UNUSED 0x01
#define HASH_TABLE_USED 0x10
#define HASH_TABLE_UNUSED 0x11

struct _policy_element
{
	int version;
	int operation;
	unsigned int up_limit;
	unsigned int lower_limit;
	unsigned long long value[2];
};

struct _policy_table
{
	int policy_table_flag;
	int policy_table_len;
	int used_len;
	int current_used_policy;
	struct _policy_element p_element[0];
};

int policy_init();
int add_policy(struct _policy_table *policy_table, struct _policy_element *p_elem);
int init_policy_table( struct _policy_table *p);
int print_policy_table( struct _policy_table *p );
int find_policy_table( struct _policy_table *p ,int key);
int delete_policy_table( struct _policy_table *p );
struct policy_elment *find_hash_table( struct _policy_hash_table *p, int key );

#endif
