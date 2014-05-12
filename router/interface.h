#ifndef __INTERFACE_H__
#     define __INTERFACE_H__

#define IFNAMSIZ 20

#define Q_STATE 1
#define Q_TYPE  2
#define Q_NAME  3
#define Q_CTID  4
#define Q_RID   5
#define Q_NID   6
#define Q_IPADDR    7
#define Q_HWADDR    8    

//access eth state
enum if_state{
                 DOWN = 0,
                 UP = 1
};
enum if_type
{
                 ACCESS = 0 , BACKBONE = 1, UNKNOWN = 2
};

struct neighbor{
    u32 rid;
    struct in_addr router_addr;
    struct in_addr agent_addr;
};

#define ROUTER_OSPF 1
#define ROUTER_BGP  2
#define BGP_EBGP 1
#define BGP_IBGP 2
struct proto_info{
    u32 proto_type;
union{
    u32 areaid;
	u32 asid;
} u;
	u32 bgp_type;// ebgp :1   ibgp:2
    u32 cost;
    void *info;
};

struct rinterface{
    enum if_type type;
    enum if_state op_state;
    enum if_state hw_state;
    char name[IFNAMSIZ+1];
    u32 ctl_id;
    struct proto_info *proto_info;
    struct neighbor *neighbor;
    struct interface* details;
    void *vinterface;
};

#define CONFIGFILE "interface.cfg"
void get_interface_info_from_snmp(char* routerip, char** buff, int* len);
int update_interface_from_snmp(char *routerip);

#endif /* __INTERFACE_H__ */
