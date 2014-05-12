#ifndef VTY_H
#define VTY_H

int vty_init();
int vty_server_start();
extern struct conf pma_conf;

#define VTY_PORT 8060

#define CMD_

#define CMD_SHOW		"show"
#define CMD_SET			"set"
#define CMD_EXIT		"exit"

#define CMD_COMM_TYPE	"comm"
#define CMD_PMSIP		"pmsip"
#define CMD_PMSPORT		"pmsport"
#define CMD_ALCAIP		"alcaip"
#define CMD_ALCAPORT	"alcaport"
#define CMD_LOGSRVIP	"logsrvip"
#define CMD_LOGSRVPORT	"logsrvport"
#define CMD_ROUTERIP	"routerip"
#define CMD_LOCALIP	    "localip"
#define CMD_LOCALMASK	"localmask"
#define CMD_OUTSIDE		"outside"
#define CMD_HELLO_H		"hello_high"
#define CMD_HELLO_L		"hello_low"
#define CMD_DEAD_H		"dead_high"
#define CMD_DEAD_L		"dead_low"
#define CMD_SNAP		"snap"
#define CMD_POLICY		"policy"
#define CMD_OSPF_HELLO	"ospf_hello"
#define CMD_OSPF_DEAD	"ospf_dead"

#define SET_HELP_STR	"set comm\n\
\tpmsip\n\
\tpmsport\n\
\talcaip\n\
\talcaport\n\
\tlogsrvip\n\
\tlogsrvport\n\
\thello_high\n\
\thello_low\n\
\tdead_high\n\
\tdead_low\n\
\trouterip\n\
\tlocalip\n\
\tlocalmask\n\
\toutside\n\
\tsnap\n\
\tpolicy\n\
\tospf_hello\n\
\tospf_dead\n"
#define SET_BM_COMM		"set comm ?(0:Alca Disable,1:Alca Enable,2:Both)\n"
#define SET_BM_PMSIP	"set pmsip ?(x.x.x.x PMS server's ip address)\n"
#define SET_BM_PORT		"set pmsport ?(int, the number port)\n"
#define SET_BM_ALCA_IP  "set alcaip ?(x.x.x.x ALCA's ip address)\n"
#define SET_BM_ALCA_PORT "set alcaport ?(int, the number alca port)\n"
#define SET_LOGSRV_IP  "set logsrvip ?(x.x.x.x LOGSRV's ip address)\n"
#define SET_LOGSRV_PORT "set logsrvport ?(int, the number LOGSRV port)\n"
#define SET_IC_HELLO_H	"set hello_high ?(high timeval(x*100ms)[1-99])\n"
#define SET_IC_HELLO_L	"set hello_low ?(low timeval(x*100ms)[1-399])\n"
#define SET_IC_DEAD_H	"set dead_high ? ?(high timeval(x*100ms)[1-399])\n"
#define SET_IC_DEAD_L	"set dead_low ? ?(high timeval(x*100ms)[1-1599])\n"
#define SET_DBM_SNAP	"set snap ?(snapshoot send timeval (second))\n"
#define SET_DBM_POLICY  "set policy ?(not used yet)\n"
#define SET_PEA_HELLO	"set ospf_hello ?(hello,second)\n"
#define SET_PEA_DEAD	"set ospf_dead ?(dead,second)\n"
#define SET_ROUTERIP    "set routerip ?(x.x.x.x the router ip pma controlled)\n"
#define SET_LOCALIP		"set localip ?(x.x.x.x the local ip of pma)\n"
#define SET_LOCALMASK	"set localmask ?(x.x.x.x the local mask of pma)\n"
#define SET_OUTSIDE		"set outside ?(int, 0 present false, 1 present true)\n"

#define CMD_EQ(x,y) (strcmp((x),(y))==0)

typedef int (*value_setter)(void* args);

typedef struct cmd_string{
	char* token;
	char* next;
} CMString;

typedef struct cmd_message{
	char* cmd;
	char* help_info;
	void* value;
	value_setter func_ptr;
	struct cmd_message* next;
} CMsg;



int set_bm_commtype(void* args);
int set_bm_pmserverip(void* args);
int set_bm_pmserverport(void* args);
int set_bm_alcaaddress(void* args);
int set_bm_alcaport(void* args);
int set_bm_logsrvaddress(void* args);
int set_bm_logsrvport(void* args);

int set_routerip(void* args);
int set_localip(void *args);
int set_localmask(void *args);
int set_outside(void *args);
int set_ic_hello_h_timeval(void* args);
int set_ic_hello_l_timeval(void* args);

int set_ic_dead_h_timeval(void* args);
int set_ic_dead_l_timeval(void* args);

int set_dbm_snapshoot_timeval(void* args);
int set_dbm_policytype(void* args);
int set_pea_ospf_hello(void* args);
int set_pea_ospf_dead(void* args);
int show_total_module(void* args);

int show_bm_config(void* args);

int show_ic_config(void* args);

int show_ic_interface(void* args);

int show_ic_routertable(void* args);

int show_ic_neighbor(void* args);

int show_dbm_config(void* args);
int show_dbm_policy_dm(void* args);
int show_dbm_lsdb(void* args);
int show_dbm_lidb(void* args);
int show_spf_history(void* args);

int show_pea_history(void* args);
#endif
