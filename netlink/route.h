#ifndef ROUTE_H

#define ROUTE_H

#define FRESH_STATUS 3
#define OLD_STATUS 2
#define NEW_STATUS 1


#define ROUTE_TYPE_LOCAL 0
#define ROUTE_TYPE_REMOTE 1

/* IPv4 prefix structure. */
struct prefix_ipv4
{
  u8 family;
  u8 prefixlen;
  struct in_addr prefix __attribute__ ((aligned (8)));
};	


struct route{
	struct prefix_ipv4 prefix;
	struct in_addr gateway;
	int interface_id;
	int metric;
	int type;
	int dirty;//flag is or not modified
	struct route* next;
};

struct route* find_before_route(struct route* rtable, struct route rt);
int show_route_table();
#endif /* end of include guard: ROUTE_H */
