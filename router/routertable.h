#ifndef ROUTERTABLE_H

#define ROUTERTABLE_H

#define NETLINK_ROUTE	1
#define SNMP_ROUTE		2


struct route_table{
	char routerip[24];
	int route_table_len;
	struct route* routes;
	int acquire_type;
};

int route_table_get(struct route_table * rt);

#endif /* end of include guard: ROUTERTABLE_H */
