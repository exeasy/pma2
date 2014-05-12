#ifndef PMA_API_H_
#define PMA_API_H_
/**
 * ip community
 */
#define HUAWEI_IP "192.168.1.122"
#define QUAGGA_IP "192.168.1.223"
#define COMMUNITY "public"
#define SLEEPTIME 3
/**
 * 端口信息结构体，用于存储获取的端口信息
 */
typedef struct ifTable {
	char ifdex[5];
	char ifDescr[22];
	char ifDescrDex[3];
	char ifBandwidth[15];
	char ip[16];
	char physAddress[18];
	char netMask[16];
	char areaId[16];
	char ospfIfHelloInterval[20];
	char ospfIfDeadInterval[20];
	char ifAdminStatus[15];
	char ifOperStatus[15];
	char ospfCost[5];
	char ospfNbrIp[16];
	char nbrRouteId[16];
	long long inrate;
	long long outrate;
	long long inoctets;
	long long outoctets;
	long time;
	struct ifTable * next;
} ifTable;

typedef struct ifRate {
	char ifdex[5];
	char ifDescr[22];
	int inrate;
	int outrate;
	struct ifRate * next;
} ifRate;

typedef struct ipRouteTable {
	char ipRouteDest[16];
	char ipRouteIfIndex[5];
	char ipRouteMetric[5];
	char ipRouteNextHop[16];
	char ipRouteType[10];
	char ipRouteProto[10];
	char ipRouteAge[10];
	char ipRouteMask[16];
	struct ipRouteTable * next;
} ipRouteTable;
/**example
 *  ipRouteDest ipRouteIfIndex ipRouteMetric1  ipRouteNextHop ipRouteType ipRouteProto ipRouteAge     ipRouteMask
      10.1.111.0              4            349  192.168.17.1    indirect         ospf    1748894   255.255.255.0
      10.5.114.0              5            332  192.168.58.5    indirect         ospf    1749022   255.255.255.0
       127.0.0.0              1              0     127.0.0.1      direct        local    1836895       255.0.0.0
 */

/*
 * 获取IFTABLE，IPADDRTABLE，OSPFIFTABLE，整理到iftable的链表中。注意链表的第一节点没有赋值，数据从链表的第二节点开始。
 */
int getIfInfos(char * routerIp, ifTable ** head);

int getBgpIfInfos(char * routerIp, ifTable ** head);

int getBgpLocalAs(char* routerIp, char *As);

//获取接口的当前发送接受速率，单位kbps .通过snmp获取两次接口通过的字节数来计算 间隔时间为SLEEPTIME
int getIfRate(char *routerIp, ifTable *head, int sample_time);
/**
 * print getIfInfos获得的链表（从链表第一节开始print）
 */
void printIftable(const ifTable *table);

/**
 * 获取iproutetable，head为返回链表头，内容从第二个节点开始
 */
int getIpRouteTable(char *routerIp, ipRouteTable **head );

int getIpForwardTable(char *routerIp, ipRouteTable **head);

void printIpRouteTable(const ipRouteTable *table);
/**
 * 获取路由器ospf id
 * @routerIp 路由器ip地址
 * @id 用于返回得到的路由器id
 */
int getOspfRouterId(char * routerIp, char * Id);

/**
 * 获取路由器spf计算次数
 */
int getRouterSpfCount(char *routerIp, char *areaid);

/**
 * 向路由器添加一条静态路由, 成功返回0。
 */
int addStaticRoute(char * routerIp, char * desIp, char *mask, char * nextHop);

/**
 *  删除静态路由
 */
int removeStaticRoute(char * routerIp, char * desIp, char *mask, char * nextHop);

/**
 * 设置路由器端口的hello定时器值，成功返回0.
 */
int setHelloInteval(char * routerIp, char *ethId, char *value);

/**
 * 设置路由器端口的dead定时器值，成功返回0.
 */
int setDeadInteval(char * routerIp, char *ethId, char *value);

/**
 * 设置路由器端口的Retransmit定时器值，成功返回0.
 */
int setRetransmitInteval(char * routerIp, char *ethId, char *value);

/**
 * 设置路由器端口的cost值，成功返回0.
 */
int setCost(char * routerIp, char *ethId, char *value);
/*
 * 获取接口的当前发送接受速率，单位kbps;通过snmp获取两次接口通过的字节数来计算 间隔时间为SLEEPTIME
 * inrate进口方向速率数组；inrate进口方向速率数组；ifnum接口数量
 */
//int getIfRate(char * routerIp, ifTable *head);

/**
 * peer route-update-interval (BGP) 	--huawei
 * neighbor advertisement-interval		--quagga

命令功能

peer route-update-interval命令用来配置向对等体（组）发送同一路由的路由更新报文（Update报文）的时间间隔。
undo peer route-update-interval命令用来恢复发送路由更新的时间间隔为缺省值。
缺省情况下，IBGP对等体的路由更新时间间隔为15秒，EBGP对等体的路由更新时间间隔为30秒。
命令格式
peer { group-name | ipv4-address } route-update-interval interval
undo peer { group-name | ipv4-address } route-update-interval

成功返回0
 */
int setBgpPeerUpdateInterval(char *routerIp, char *bgpAreaId, char *peerIp, int interval);

#endif /* PMA_API_H_ */
