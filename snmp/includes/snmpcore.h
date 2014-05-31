#ifndef SNMPCORE_H

#define SNMPCORE_H

#define MAX_ITEM_LEN 1024
#define MAX_IFNAME_SIZE 100
#define MAX_ADDRESS_LEN 20
#define TIME_STR_LEN 20
#define GETSESSION(x) (snmpsession*)s;
#define GETELEMENT(p,i) (((char**)p)+i);
#define GETNEXTPTR(p,i) (((char**)(p))+i);

typedef unsigned int session_id_t;

typedef int (*callbackptr)(void*);

typedef struct snmpsession{
	char ip[32];
	char community[10];
}snmpsession;

typedef	struct itemopt{
	char* itemhead;
	const char* oids;
}itemopt;

typedef struct tableopt{
	char* tablehead;
	const char* oids;
	size_t type_size;
	size_t column;
	const int* mapper;
}tableopt;

struct InterfaceTable {
	char* ifindex;
	char* ifdescr;
	char* iftype;
	char* ifmtu;
	char* ifspeed;
	char* ifphysaddress;
	char* ifadminstatus;
	char* ifoperstatus;
	char* ifinoctets;
	char* ifoutoctets;
	struct InterfaceTable* next;
};

struct IpAddrTable{
	char* ipaddr;
	char* ifindex;
	char* ipmask;
	char* bcastaddr;
	struct IpAddrTable* next;
};

struct IpRouteTable {
	char* dest;
	char* mask;
	char* tos;
	char* nexthop;
	char* ifindex;
	char* type;
	char* proto;
	char* nextas;
	char* metric;
	char* status;
	struct IpRouteTable* next;
} ;

struct ospfAreaTable {
	char* areaid;
	char* spfruns;
	char* lsacount;
	char* status;
	struct ospfAreaTable* next;
} ;

struct ospfIfTable {
	char* ipaddress;
	char* areaid;
	char* type;
	char* adminstatus;
	char* transit;
	char* retransit;
	char* hello;
	char* dead;
	char* pool;
	char* state;//dr or bdr
	char* drouter;
	char* bdrouter;
	char* ifevent;
	char* status;
	struct ospfIfTable* next;
};

struct ospfIfMetricTable {
	char* ipaddress;
	char* metric;
	char* status;
	struct ospfIfMetricTable* next;
};

struct ospfNeighborTable {
	char* nbrip;
	char* nbrid;
	char* state;
	char* events;
	char* status;
	struct ospfNeighborTable * next;
};

struct bgpPeerTable {
	char* identifier;
	char* peerstate;
	char* adminstatus;
	char* localaddr;
	char* localport;
	char* peeraddr;
	char* peerport;
	char* remoteas;
	char* inupdates;
	char* outupdates;
	char* inmessages;
	char* outmessages;
	char* estabtime;
	char* conretrytime;
	char* holdtime;
	char* keepalive;
	char* holdtime_conf;
	char* keepalive_conf;
	char* minasorigination;
	char* minrouteadvertise;
	char* updateelapsed;
	struct bgpPeerTable* next;
} ;

struct bgpRecvPathAttrTable {
	char* peer;
	char* destnetwork;
	char* origin;
	char* aspath;
	char* nexthop;
	char* inter_as_metric;
	struct bgpRecvPathArrtTable* next;
};

struct bgpPathAttrTable {
	char* peer;
	char* addr_prefix_len;
	char* addr_prefix;
	char* origin;
	char* as_path_segment;
	char* nexthop;
	char* multi_exit_disc;
	char* local_pref;
	char* atomic_aggregate;
	char* aggregator_as;
	char* aggregator_addr;
	char* calc_localpref;
	char* best;
	struct bgpPathArrtTable* next;
} ;



int get_ospf_routerid(char* routerid);
int get_bgp_routerid(char* routerid);
int get_bgp_asid(char* asid);
int get_bgp_peer_table(callbackptr callback);
int get_bgp_path_table(callbackptr callback);
int get_if_table(callbackptr callback);
int get_ipaddr_table(callbackptr callback);
int get_iproute_table(callbackptr callback);
int get_ospf_area_table(callbackptr callback);
int get_ospf_ifmetric_table(callbackptr callback);
int get_ospf_if_table(callbackptr callback);
int get_ospf_neighbor_table(callbackptr callback);

#endif /* end of includse guard: SNMPCORE_H */
