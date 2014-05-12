#include <utils/utils.h>
#include <utils/common.h>
#include <netlink/netlink.h>
#include <netlink/route.h>
#include <router/routertable.h>

int main()
{
	struct route_table* rtable 
		= malloc_z(struct route_table);
	rtable->acquire_type = SNMP_ROUTE;
	rtable->routes = malloc_z(struct route);
	sprintf(rtable->routerip, "192.168.3.1");
	while(1)
	{
		route_table_get(rtable);
		if (1 == watch_route_change(rtable->routes))
		{
			DEBUG(INFO,"ROUTE DETECTED");
		}
		refresh_route_status(rtable->routes);
		usleep(1000);
	}
}
