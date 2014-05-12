#ifndef __LSD_IF_H__
#     define __LSD_IF_H__

#define IFNAMSIZ 20
typedef  unsigned int  id_t;
enum eth_type{
    ACCESS_ETH = 0,
    BACKBONE_ETH = 1};
enum eth_state{
    ETH_DOWN = 0,
    ETH_UP = 1 };

struct neighbor_pma{
    u32 rid;
    struct in_addr pma_ctl_addr;
    struct in_addr router_addr;
};


struct common_eth{
    u32 ifid;
    enum eth_state state;
    enum eth_type type;
};

struct backbone_eth  //BackBone Interface
{
    struct common_eth eth_info;
#define _ifid eth_info.ifid
#define _state eth_info.state
#define _type eth_info.type
    struct neighbor_pma neighbor_pma; 
    struct timeval adjust_time; //
    struct autonomous_zone* az; //whic area this if belongs to
    struct rinterface * interface; //the detail infomation of the interface
    void* exchange_master; //ctrl of the exchange protrocl
    void* flood_master; //ctrl of the flood protocl
    void* hello_master; //ctrl of the hello protocl
    struct backbone_eth* next; //next interface
};

struct access_eth
{
    struct common_eth eth_info;
#define _ifid eth_info.ifid
#define _state eth_info.state
#define _type eth_info.type
    struct rinterface* interface;
    struct access_eth* next;
};

struct lsd_router //we use this struct to record the area and access interface 
{
     struct autonomous_zone* azs; //
     struct access_eth* accesses;
};

struct autonomous_zone //area in ospf 
{
     struct autonomous_zone* next; 
     u32 r_id;  //routerid exposed to the area
     u32 az_id;  //area id
     id_t lsdb; // lsdb of the area
     id_t lidb; //lidb of the area //2013.04.09  not used in ic
     struct backbone_eth* backbones;//backbone interfaces
};

//int set_interface_state(int if_id,int state); //set the state of if
//
int eth_state_update(struct common_eth* eth); //update the state of if
int interface_up(struct thread *thread); 
int interface_down(struct thread *thread);
void backbone_eth_add(struct autonomous_zone* azone,struct backbone_eth *backbone_eth);
void access_eth_add(struct access_eth *access_eth);
void autonomous_zone_add(struct autonomous_zone *az);
struct autonomous_zone* find_autonomous_zone(int area_id);
u32 get_area_id_by_lsdb_handle(id_t lsdb_handle);
int  get_address_of_access( struct access_eth* eth, struct in_addr* addr);
int  get_address_of_backbone( struct backbone_eth* eth, struct in_addr* addr);
int * get_all_areas(int * count);
//struct autonomous_zone* get_autonomous_zone_by_id(u_int32_t area_id);
//struct backbone_eth * get_backbone_eth_by_zone_and_id(struct autonomous_zone* az,int interface_id);
//struct backbone_eth * get_backbone_eth_by_id(int interface_id);
//struct backbone_eth * get_backbone_eth_by_zone_and_name(struct autonomous_zone* az, const char *if_name);
//struct backbone_eth * get_backbone_eth_by_name(const char *if_name);
//struct access_eth * get_access_eth_by_id(int interface_id);
//struct access_eth * get_access_eth_by_name(const char *interface_name);
//
//u_int32_t get_router_id_by_handle(id_t lsdb_handle);//Added by Ray
//
//int get_interface_address(struct interface* interface, struct in_addr* addr);
#define get_backbone_eth_area(eth) ({struct autonomous_zone* az = (eth)->az; az;})
#define lsdb_get_eth_handle(eth)	({ id_t lsdb = (get_backbone_eth_area(eth))->lsdb; lsdb;})
#define lidb_get_eth_handle(eth)	({ id_t lidb = (get_backbone_eth_area(eth))->lidb; lidb;})
#define lsdb_get_area_handle(area_id) ({ id_t lsdb = (get_autonomous_zone_by_id(area_id))->lsdb; lsdb;})
//int load_data();
//int load_if_list();
//int test_interface();
//int LoadConfigurationFromFile();
//int UpdateInterfaceFromSnmp();
//int LoadInterfaceFromSnmp();
#endif /* __LSD_IF_H__ */
