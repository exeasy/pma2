#ifndef __ROUTER_H__
#     define __ROUTER_H__

#define OSPF_ROUTER 1
#define BGP_ROUTER 2

struct router{
    u32 routerid;
    struct in_addr routerip;
    u32 agentid;
	u32 as_id;
	u32 type;
    u8 inited;
    struct list* iflist;
};
    
#endif /* __ROUTER_H__ */
