#include <utils/common.h>
#include <utils/utils.h>
#include <lib/linklist.h>
#include "router.h"

struct router localrouter;

struct router *create_router();


struct router *get_self_router();


int init_local_router(int routerid, int pmaid, int routerip, int type)
{
    localrouter.inited = 0;
    localrouter.routerid = routerid;
    localrouter.agentid = pmaid;
    localrouter.routerip.s_addr  = routerip;
	localrouter.type = type;
    localrouter.inited = 1;
}


