#ifndef PMA_CONF_H
#define PMA_CONF_H

struct ic_conf{
	struct in_addr router_ip;
	struct in_addr local_ip;
	struct in_addr netmask;
	int outside;
	int device_type; // 11: Quagga+ospf 12: Quagga+bgp 13: Quagga+ospf+bgp 21: Huawei+ospf 22: Huawei+bgp  23: Huawei+ospf+bgp
	int pma_id;
	int if_id;
	unsigned int h_hello_val;
	unsigned int h_dead_val;
	unsigned int l_hello_val;
	unsigned int l_dead_val;
};

struct dbm_conf{
	int snapshoot_timeval;
	int policy_type;
	int pma_id;
	int router_id;
};

struct pea_conf{
	struct in_addr router_ip;
	int pma_id;
	int fast_mpls;
	int device_type; // 11: Quagga+ospf 12: Quagga+bgp 13: Quagga+ospf+bgp 21: Huawei+ospf 22: Huawei+bgp  23: Huawei+ospf+bgp
};

struct conf{
	int version;
	int pma_id;
	int as_num;//as id
	int server_port;
	int alca_port;
	int rltm_port;
	int listen_port;
	int logsrv_port;
	int device_type; // 11: Quagga+ospf 12: Quagga+bgp 13: Quagga+ospf+bgp 21: Huawei+ospf 22: Huawei+bgp  23: Huawei+ospf+bgp
	int comm_type;  //  0,no alca , 1,two method, 2,only alca 
	char alca_ip[46];//ALCA IP ADDRESS
	char server_ip[46];
	char rltm_ip[46];
	char logsrv_ip[46];
	struct ic_conf ic_config;
	struct dbm_conf dbm_config;
	struct pea_conf pea_config;
};
extern struct conf pma_conf;
extern char *conf_file;
extern int conf_init();
extern int get_protocol_type();
extern int get_device_type();
extern int get_comm_type(void);
extern int get_version();
char *get_alca_address();
int get_alca_port();
char *get_logsrv_address();
int get_logsrv_port();
char* get_rltm_address(void);
int get_rltm_port();
extern int get_local_ip();
extern int get_timeval();
extern int get_pma_id();
extern int get_policy_type();
extern char *get_server_address();
extern int get_server_port();
extern int get_listen_port();
extern int get_router_id(char *ip);

#endif
