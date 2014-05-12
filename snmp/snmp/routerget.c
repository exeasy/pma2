/*
 * getAll.c
 *
 *  Created on: Jan 12, 2013
 *      Author: liupengzhan
 */
//ifTable:ifdex ifdescr physAddress status
#include"oids.h"
#include "pma_api.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

int stringToInt(char *string);
long long int stringTollInt(char *string);
void sleeptime(int second);

void printIftable(const ifTable *table) {
	ifTable *it = table;
	while (it) {
		printf("ifdex = %s\n", it->ifdex);
		printf("ifDescr = %s \n", it->ifDescr);
		printf("ifDescrDex = %s \n", it->ifDescrDex);
		printf("ifBandwidth = %s \n", it->ifBandwidth);
		printf("ip = %s\n", it->ip);
		printf("pthysAssress = %s\n", it->physAddress);
		printf("netMask = %s\n", it->netMask);
		printf("areaId = %s\n", it->areaId);
		printf("helloInterval = %s\n", it->ospfIfHelloInterval);
		printf("deadInterval = %s\n", it->ospfIfDeadInterval);
		printf("ifAdminStatus = %s\n", it->ifAdminStatus);
		printf("ifOperStatus = %s\n", it->ifOperStatus);
		printf("ospfcost = %s\n", it->ospfCost);
		printf("nbrRouteId= %s\n", it->nbrRouteId);
		printf("ospfNbrIp= %s\n", it->ospfNbrIp);
		printf("inrate= %lld\n", it->inrate);
		printf("outrate= %lld\n", it->outrate);
		it = it->next;
		printf("\n");
	}
}

void printIpRouteTable(const ipRouteTable *table) {
	ipRouteTable *it = table;
	while (it) {
		printf("ipRouteDest = %s\n", it->ipRouteDest);
		printf("ipRouteIfIndex = %s \n", it->ipRouteIfIndex);
		printf("ipRouteMetric = %s\n", it->ipRouteMetric);
		printf("ipRouteNextHop = %s\n", it->ipRouteNextHop);
		printf("ipRouteType = %s\n", it->ipRouteType);
		printf("ipRouteProto = %s\n", it->ipRouteProto);
		printf("ipRouteAge = %s\n", it->ipRouteAge);
		printf("ipRouteMask = %s\n", it->ipRouteMask);
		it = it->next;
		printf("\n");
	}
}
void freeIftable(ifTable *head) {
	ifTable *ita = head;
	ifTable *itb = NULL;
	while (ita) {
		itb = ita->next;
		free(ita);
		ita = itb;
	}
	head = NULL;
}
void freeIpRouteTable(ipRouteTable *head) {
	ipRouteTable *ita = head;
	ipRouteTable *itb = NULL;
	while (ita) {
		itb = ita->next;
		free(ita);
		ita = itb;
	}
	head = NULL;
}
/**
 * 判断2个ip加子网掩码 是否属于某一网段；
 */
int isSameAddrSegment(char * ip1, char * ip2, char * netmask) {
	if (ip1 == NULL || ip2 == NULL || netmask == NULL) {
		return -1;
	}
	struct in_addr addr_ip1;
	struct in_addr addr_ip2;
	struct in_addr addr_netmask;
	struct in_addr addr_ipseg1;
	struct in_addr addr_ipseg2;
	char ip_seg1[16];
	char ip_seg2[16];
	memset(&addr_ip1, 0, sizeof(struct in_addr));
	memset(&addr_ip2, 0, sizeof(struct in_addr));
	memset(&addr_netmask, 0, sizeof(struct in_addr));
	memset(&addr_ipseg1, 0, sizeof(struct in_addr));
	memset(&addr_ipseg2, 0, sizeof(struct in_addr));

	inet_aton(ip1, &addr_ip1);
	inet_aton(ip2, &addr_ip2);
	inet_aton(netmask, &addr_netmask);
	addr_ipseg1.s_addr = addr_ip1.s_addr & addr_netmask.s_addr;
	addr_ipseg2.s_addr = addr_ip2.s_addr & addr_netmask.s_addr;
	printf("%s\n",inet_ntoa(addr_ipseg1));
	strcpy(ip_seg1, inet_ntoa(addr_ipseg1));
	strcpy(ip_seg2, inet_ntoa(addr_ipseg2));
	return strcmp(ip_seg1, ip_seg2);
	//    printf("the segment of %s is %s\n", ip, ip_seg);
	//    return 0;
}
int initStructIfNode(ifTable * next) {
	memset(next->areaId, '\0', sizeof(next->areaId));
	memset(next->ifAdminStatus, '\0', sizeof(next->ifAdminStatus));
	memset(next->ifDescr, '\0', sizeof(next->ifDescr));
	memset(next->ifDescrDex, '\0', sizeof(next->ifDescrDex));
	memset(next->ifBandwidth, '\0', sizeof(next->ifBandwidth));
	memset(next->ifOperStatus, '\0', sizeof(next->ifOperStatus));
	memset(next->ifdex, '\0', sizeof(next->ifdex));
	memset(next->ip, '\0', sizeof(next->ip));
	memset(next->nbrRouteId, '\0', sizeof(next->nbrRouteId));
	memset(next->netMask, '\0', sizeof(next->netMask));
	memset(next->ospfCost, '\0', sizeof(next->ospfCost));
	memset(next->ospfIfDeadInterval, '\0', sizeof(next->ospfIfDeadInterval));
	memset(next->ospfIfHelloInterval, '\0', sizeof(next->ospfIfHelloInterval));
	memset(next->ospfNbrIp, '\0', sizeof(next->ospfNbrIp));
	memset(next->physAddress, '\0', sizeof(next->physAddress));
	next->inrate = 0;
	next->outrate = 0;
	return 0;
}
/**
 * 获取iftable并初始化链表，分配空间。将获取的内容相应的赋值给iptable结构体
 */
int getIfTable(char * routerIp, ifTable ** head) {
	if (routerIp == NULL) {
		return -1;
	}
	int fields, entries, result;
	char** data = NULL;
	char **dp = NULL;
	ifTable *pre, *next;//*head,
	int entry, field;

	next = (ifTable *) malloc(sizeof(ifTable));
	*head = next;
	pre = next;
	(*head)->next = NULL;

	long t = time(NULL);
	result = snmptable(routerIp, COMMUNITY, IFTABLE, &entries, &fields, &data);

	if (result != 0) {
		printf("get IFTABLE error\n");
		return -1;
	} else {
		printf("ZZZZZZZZZZ5%d\n",data);
		dp = data;
		for (entry = 0; entry < entries; entry++) {
			if ((strstr(dp[1], "eth") == NULL && strstr(dp[1],
					"GigabitEthernet") == NULL)) {
				dp = dp + fields;
				continue;
			}
			next = (ifTable *) malloc(sizeof(ifTable));

			initStructIfNode(next);

			pre->next = next;
			strcpy(next->ifdex, dp[0] ? dp[0] : "?");
			strcpy(next->ifDescr, dp[1] ? dp[1] : "?");
			strcpy(next->ifBandwidth, dp[4] ? dp[4] : "?");
			if(strcmp(next->ifBandwidth,"0")==0){
				strcpy(next->ifBandwidth, "100000000");
			}
			strcpy(next->ifAdminStatus, dp[6] ? dp[6] : "?");
			strcpy(next->ifOperStatus, dp[7] ? dp[7] : "?");
			strcpy(next->physAddress, dp[5] ? dp[5] : "?");
			if (strlen(next->ifDescr) > 19) {//huawei
				strncpy(next->ifDescrDex, next->ifDescr + 19, strlen(
						next->ifDescr) - 19 + 1);
			} else {//quagga
				strncpy(next->ifDescrDex, next->ifDescr + 3, strlen(
						next->ifDescr) - 3 + 1);
			}
			next->time = t;
			next->inoctets = stringTollInt(dp[9]);
			next->outoctets = stringTollInt(dp[15]);
			next->next = NULL;
			pre = next;
			dp = dp + fields;
		}

		free_data(data, entries, fields);
	}

	//获取IPADDRTABLE，将获取的内容相应的赋值给iptable结构体
	result = snmptable(routerIp, COMMUNITY, IPADDRTABLE, &entries, &fields,
			&data);
	if (result != 0) {
		printf("get IPADDRTABLE error\n");
		return -1;
	} else {
		dp = data;
		for (next = (*head)->next; next; next = next->next) {
			dp = data;
			for (entry = 0; entry < entries; entry++) {
				if (strcmp(next->ifdex, dp[1]) == 0) {
					strcpy(next->ip, dp[0] ? dp[0] : "?");
					strcpy(next->netMask, dp[2] ? dp[2] : "?");
					//				printf("[%i]:%s\n", entry + 1, dp[0] ? dp[0] : "?");
					break;
				}
				dp = dp + fields;
			}
		}
		free_data(data, entries, fields);
	}
	printf("get iftable end\n");
	return 0;

}
/*
 * 获取IFTABLE，IPADDRTABLE，OSPFIFTABLE，整理到iftable的链表中。head为链表头注意链表的第一节点没有赋值，数据从链表的第二节点开始。
 */
int getIfInfos(char * routerIp, ifTable ** head) {
	if (routerIp == NULL) {
		return -1;
	}
	int fields, entries, result;
	char** data = NULL;
	char **dp = NULL;
	ifTable *pre, *next;//*head,
	int entry, field;

	next = (ifTable *) malloc(sizeof(ifTable));
	*head = next;
	pre = next;
	(*head)->next = NULL;

	/**
	 * 获取iftable并初始化链表，分配空间。将获取的内容相应的赋值给iptable结构体
	 */
	long t = time(NULL);
	result = snmptable(routerIp, COMMUNITY, IFTABLE, &entries, &fields, &data);

	if (result != 0) {
		printf("get IFTABLE error\n");
	} else {
		dp = data;
		for (entry = 0; entry < entries; entry++) {
			if ((strstr(dp[1], "eth") == NULL && strstr(dp[1],
					"GigabitEthernet") == NULL)) {
				dp = dp + fields;
				continue;
			}
			next = (ifTable *) malloc(sizeof(ifTable));

			initStructIfNode(next);

			pre->next = next;
			strcpy(next->ifdex, dp[0] ? dp[0] : "?");
			strcpy(next->ifDescr, dp[1] ? dp[1] : "?");
			strcpy(next->ifBandwidth, dp[4] ? dp[4] : "?");
			if(strcmp(next->ifBandwidth,"0")==0){
				strcpy(next->ifBandwidth, "100000000");
			}
			strcpy(next->ifAdminStatus, dp[6] ? dp[6] : "?");
			strcpy(next->ifOperStatus, dp[7] ? dp[7] : "?");
			strcpy(next->physAddress, dp[5] ? dp[5] : "?");
			if (strlen(next->ifDescr) > 19) {//huawei
				strncpy(next->ifDescrDex, next->ifDescr + 19, strlen(
						next->ifDescr) - 19 + 1);
			} else {//quagga
				strncpy(next->ifDescrDex, next->ifDescr + 3, strlen(
						next->ifDescr) - 3 + 1);
			}
			next->time = t;
			next->inoctets = stringTollInt(dp[9]);
			next->outoctets = stringTollInt(dp[15]);
			next->next = NULL;
			pre = next;
			dp = dp + fields;
		}
		free_data(data, entries, fields);
	}

	/**
	 * 获取IPADDRTABLE，将获取的内容相应的赋值给iptable结构体
	 */
	result = snmptable(routerIp, COMMUNITY, IPADDRTABLE, &entries, &fields,
			&data);
	if (result != 0) {
		printf("get IPADDRTABLE error\n");
	} else {
		dp = data;
		for (next = (*head)->next; next; next = next->next) {
			dp = data;
			for (entry = 0; entry < entries; entry++) {
				if (strcmp(next->ifdex, dp[1]) == 0) {
					strcpy(next->ip, dp[0] ? dp[0] : "?");
					strcpy(next->netMask, dp[2] ? dp[2] : "?");
					//				printf("[%i]:%s\n", entry + 1, dp[0] ? dp[0] : "?");
					break;
				}
				dp = dp + fields;
			}
		}
		free_data(data, entries, fields);
	}

	/**
	 * 获取OSPFIFTABLE，将获取的内容相应的赋值给iptable结构体
	 */
	result = snmptable(routerIp, COMMUNITY, OSPFIFTABLE, &entries, &fields,
			&data);
	if (result != 0) {
		printf("get OSPFIFTABLE error\n");
	} else {
		for (next = (*head)->next; next; next = next->next) {
			dp = data;
			for (entry = 0; entry < entries; entry++) {
				if (strcmp(next->ip, dp[0]) == 0) {
					strcpy(next->areaId, dp[2] ? dp[2] : "?");
					strcpy(next->ospfIfHelloInterval, dp[8] ? dp[8] : "?");
					strcpy(next->ospfIfDeadInterval, dp[9] ? dp[9] : "?");
					//				printf("[%i]:%s\n", entry + 1, dp[0] ? dp[0] : "?");
					break;
				}
				dp = dp + fields;
			}
		}
		free_data(data, entries, fields);
	}

	/**
	 * 获取OSPFIFMETRICTABLE，将获取链路cost值相应的赋值给iptable结构体
	 */
	result = snmptable(routerIp, COMMUNITY, OSPFIFMETRICTABLE, &entries,
			&fields, &data);
	if (result != 0) {
		printf("get OSPFIFMETRICTABLE error\n");
	} else {
		//	for (next = (*head)->next; next; next = next->next) {
		//		dp = data;
		//		for (entry = 0; entry < entries; entry++) {
		//			if (strcmp(next->ip, dp[0]) == 0) {
		//				strcpy(next->ospfCost, dp[3] ? dp[3] : "?");
		//				//					printf("[%i]:%s\n", entry + 1, dp[0] ? dp[0] : "?");
		//				break;
		//			}
		//			dp = dp + fields;
		//		}
		//	}

		dp = data;
		for (entry = 0; entry < entries; entry++) {
			for (next = (*head)->next; next; next = next->next) {
				if (strcmp(next->ip, dp[0]) == 0) {
					strcpy(next->ospfCost, dp[3] ? dp[3] : "?");
					//					printf("[%i]:%s\n", entry + 1, dp[0] ? dp[0] : "?");
					break;
				}
			}
			dp = dp + fields;
		}
		free_data(data, entries, fields);
	}

	/**
	 * 获取OSPFNBRTABLE，将邻居ip与当前路由器端口ip比较在同一网段
	 */
	result = snmptable(routerIp, COMMUNITY, OSPFNBRTABLE, &entries, &fields,
			&data);
	if (result != 0) {
		printf("get OSPFNBRTABLE error\n");
	} else {
		for (next = (*head)->next; next; next = next->next) {
			dp = data;
			for (entry = 0; entry < entries; entry++) {
				if (next->ip != NULL && strcmp(next->ip, "") != 0
						&& isSameAddrSegment(next->ip, dp[0], next->netMask)
								== 0) {
					strcpy(next->ospfNbrIp, dp[0] ? dp[0] : "?");
					strcpy(next->nbrRouteId, dp[2] ? dp[2] : "?");
					break;
				}
				dp = dp + fields;
			}
		}
		free_data(data, entries, fields);
	}
	printf("get ALL end\n");
	return 0;

}

//获取接口的当前发送接受速率，单位kbps .通过snmp获取两次接口通过的字节数来计算 采样时间可调整，最小一秒
int getIfRate(char *routerIp, ifTable *head, int sample_time)
{
	if (routerIp == NULL || head == NULL) {
		printf("getIfRate error : head is NULL\n");
		return -1;
	}
	int fields, entries, result;
	char** data = NULL;
	char** dp = NULL;
	int entry, field;
	int ifindex = 0;
	time_t first;
	time_t second;
	long difftime = 0;
	ifTable *next = head->next;

	//第一次获取，记录当前系统时间
	first = time(NULL);
	result = snmptable(routerIp, COMMUNITY, IFTABLE, &entries, &fields, &data);
	puts("first************");
	if (result != 0) {
		printf("getIfRate error\n");
		return -1;
	} else {
		dp = data;
		for (next = head->next; next; next = next->next) {
			dp = data;
			for (entry = 0; entry < entries; entry++) {
				if (strcmp(next->ifDescr, dp[1]) == 0) {
					printf("%s %s %s\n", dp[1], dp[9], dp[15]);
					next->inrate = atoll(dp[9]);
					next->outrate = atoll(dp[15]);
					break;
				}
				dp = dp + fields;
			}
		}
		free_data(data, entries, fields);
	}
	ifindex = 0;
	dp = NULL;

	//	sleeptime(SLEEPTIME);//延迟SLEEPTIME秒
	sleep(sample_time);

	//第二次获取
	second = time(NULL);
	difftime = second - first;
	//		printf("difftime %d\n", difftime);
	result = snmptable(routerIp, COMMUNITY, IFTABLE, &entries, &fields, &data);
	puts("second************");
	if (result != 0) {
		printf("get IFTABLE error\n");
		return -1;
	} else {
		dp = data;
		for (next = head->next; next; next = next->next) {
			dp = data;
			for (entry = 0; entry < entries; entry++) {
				if (strcmp(next->ifDescr, dp[1]) == 0) {
					printf("%s %s %s\n", dp[1], dp[9], dp[15]);
					long long inrate = atoll(dp[9]);
					long long outrate = atoll(dp[15]);
					printf("%lld - %lld =  %lld\n", inrate, next->inrate,
							(inrate - next->inrate));
					printf("%lld - %lld =  %lld\n", outrate,
							next->outrate, (outrate - next->outrate));
					next->inrate = (inrate - next->inrate) * 8 / difftime;
					next->outrate = (outrate - next->outrate) * 8
							/ difftime;
					//					if (next->inrate == 0) {
					//						ifTable *node = _rltm_iftable->next;
					//						while(node){
					//							if(strcmp(node->ifDescr, next->ifDescr) == 0){
					//								next->inrate = (atoll(dp[9]) - node->inoctets)
					//																* 8 / (second - node->time);
					//								break;
					//							}
					//							node = node->next;
					//						}
					//					}
					//					if (next->outrate == 0) {
					//						ifTable *node = _rltm_iftable->next;
					//						while (node) {
					//							if (strcmp(node->ifDescr, next->ifDescr) == 0) {
					//								next->outrate = (atoll(dp[15])
					//										- node->outoctets) * 8 / (second
					//										- node->time);
					//								break;
					//							}
					//							node = node->next;
					//						}
					//					}
					break;
				}
				dp = dp + fields;
			}
		}
		free_data(data, entries, fields);
	}
	return 0;

}


/**
 * 获取路由器ospf id
 * @routerIp 路由器ip地址
 * @id 用于返回得到的路由器id
 */
int getOspfRouterId(char * routerIp, char * Id) {
	if (routerIp == NULL || Id == NULL) {
		return -1;
	}
	return snmpget(routerIp, COMMUNITY, OSPFROUTERID, Id);
}
int getBgpRouterId(char * routerIp, char * Id) {
	if (routerIp == NULL || Id == NULL) {
		return -1;
	}
	return snmpget(routerIp, COMMUNITY, BGPIDENTIFIER, Id);
}
int getBgpLocalAs(char* routerIp, char *As){
    if (routerIp == NULL || As == NULL) {
        return -1;
    }
    return snmpget(routerIp, COMMUNITY, BGPLOCALAS, As);
}
int getIpRouteTable(char *routerIp, ipRouteTable **head) {
	if (routerIp == NULL) {
		return -1;
	}
	int fields, entries, result;
	char ** data = NULL;
	char ** dp = NULL;
	ipRouteTable *pre, *next;//*head,
	int entry, field;

	next = (ipRouteTable *) malloc(sizeof(ipRouteTable));
	*head = next;
	pre = next;
	(*head)->next = NULL;

	result = snmptable(routerIp, COMMUNITY, IPROUTETABLE, &entries, &fields,
			&data);

	if (result != 0) {
		printf("get IPROUTETABLE error\n");
	}
	dp = data;
	for (entry = 0; entry < entries; entry++) {
		next = (ipRouteTable *) malloc(sizeof(ipRouteTable));

		memset(next->ipRouteDest, '\0', sizeof(next->ipRouteDest));
		memset(next->ipRouteIfIndex, '\0', sizeof(next->ipRouteIfIndex));
		memset(next->ipRouteMetric, '\0', sizeof(next->ipRouteMetric));
		memset(next->ipRouteNextHop, '\0', sizeof(next->ipRouteNextHop));
		memset(next->ipRouteType, '\0', sizeof(next->ipRouteType));
		memset(next->ipRouteProto, '\0', sizeof(next->ipRouteProto));
		memset(next->ipRouteAge, '\0', sizeof(next->ipRouteAge));
		memset(next->ipRouteMask, '\0', sizeof(next->ipRouteMask));

		pre->next = next;
		strcpy(next->ipRouteDest, dp[0] ? dp[0] : "?");
		strcpy(next->ipRouteIfIndex, dp[1] ? dp[1] : "?");
		strcpy(next->ipRouteMetric, dp[2] ? dp[2] : "?");
		strcpy(next->ipRouteNextHop, dp[6] ? dp[6] : "?");
		strcpy(next->ipRouteType, dp[7] ? dp[7] : "?");
		strcpy(next->ipRouteProto, dp[8] ? dp[8] : "?");
		strcpy(next->ipRouteAge, dp[9] ? dp[9] : "?");
		strcpy(next->ipRouteMask, dp[10] ? dp[10] : "?");
		next->next = NULL;
		pre = next;
		dp = dp + fields;
	}
	free_data(data, entries, fields);
	return 0;
}
int getIpForwardTable(char *routerIp, ipRouteTable **head) {
	if (routerIp == NULL) {
		return -1;
	}
	int fields, entries, result;
	char ** data = NULL;
	char ** dp = NULL;
	ipRouteTable *pre, *next;//*head,
	int entry, field;

	next = (ipRouteTable *) malloc(sizeof(ipRouteTable));
	*head = next;
	pre = next;
	(*head)->next = NULL;

	result = snmptable(routerIp, COMMUNITY, IPFORWARDTABLE, &entries, &fields,
			&data);

	if (result != 0) {
		printf("get IPFORWARDTABLE error\n");
	}
	dp = data;
	for (entry = 0; entry < entries; entry++) {
		next = (ipRouteTable *) malloc(sizeof(ipRouteTable));

		memset(next->ipRouteDest, '\0', sizeof(next->ipRouteDest));
		memset(next->ipRouteIfIndex, '\0', sizeof(next->ipRouteIfIndex));
		memset(next->ipRouteMetric, '\0', sizeof(next->ipRouteMetric));
		memset(next->ipRouteNextHop, '\0', sizeof(next->ipRouteNextHop));
		memset(next->ipRouteType, '\0', sizeof(next->ipRouteType));
		memset(next->ipRouteProto, '\0', sizeof(next->ipRouteProto));
		memset(next->ipRouteAge, '\0', sizeof(next->ipRouteAge));
		memset(next->ipRouteMask, '\0', sizeof(next->ipRouteMask));

		pre->next = next;
		strcpy(next->ipRouteDest, dp[0] ? dp[0] : "?");
		strcpy(next->ipRouteIfIndex, dp[4] ? dp[4] : "?");
		strcpy(next->ipRouteMetric, dp[10] ? dp[10] : "?");
		strcpy(next->ipRouteNextHop, dp[3] ? dp[3] : "?");
		strcpy(next->ipRouteType, dp[5] ? dp[5] : "?");
		strcpy(next->ipRouteProto, dp[6] ? dp[6] : "?");
		strcpy(next->ipRouteAge, dp[7] ? dp[7] : "?");
		strcpy(next->ipRouteMask, dp[1] ? dp[1] : "?");
		next->next = NULL;
		pre = next;
		dp = dp + fields;
	}
	free_data(data, entries, fields);
	return 0;
}

int getBgpIfInfos(char * routerIp, ifTable ** head) {
	if (routerIp == NULL) {
		return -1;
	}
	int fields, entries, result;
	char** data = NULL;
	char **dp = NULL;
	ifTable *pre, *next;//*head,
	int entry, field;

	next = (ifTable *) malloc(sizeof(ifTable));
	*head = next;
	pre = next;
	(*head)->next = NULL;

	/**
	 * 获取iftable并初始化链表，分配空间。将获取的内容相应的赋值给iptable结构体
	 */
	result = snmptable(routerIp, COMMUNITY, IFTABLE, &entries, &fields, &data);

	if (result != 0) {
		printf("get IFTABLE error\n");
	} else {
		dp = data;
		for (entry = 0; entry < entries; entry++) {
			if (dp[5] == NULL || strcmp(dp[5], "0") == 0 || dp[5][0] == '\0'
					|| strcmp(dp[5], "0:0:0:0:0:0") == 0 || (strstr(dp[1],
					"eth") == NULL && strstr(dp[1], "GigabitEthernet") == NULL)) {
				dp = dp + fields;
				continue;
			}
			next = (ifTable *) malloc(sizeof(ifTable));

			memset(next->areaId, '\0', sizeof(next->areaId));
			memset(next->ifAdminStatus, '\0', sizeof(next->ifAdminStatus));
			memset(next->ifDescr, '\0', sizeof(next->ifDescr));
			memset(next->ifDescrDex, '\0', sizeof(next->ifDescrDex));
			memset(next->ifBandwidth, '\0', sizeof(next->ifBandwidth));
			memset(next->ifOperStatus, '\0', sizeof(next->ifOperStatus));
			memset(next->ifdex, '\0', sizeof(next->ifdex));
			memset(next->ip, '\0', sizeof(next->ip));
			memset(next->nbrRouteId, '\0', sizeof(next->nbrRouteId));
			memset(next->netMask, '\0', sizeof(next->netMask));
			memset(next->ospfCost, '\0', sizeof(next->ospfCost));
			memset(next->ospfIfDeadInterval, '\0',
					sizeof(next->ospfIfDeadInterval));
			memset(next->ospfIfHelloInterval, '\0',
					sizeof(next->ospfIfHelloInterval));
			memset(next->ospfNbrIp, '\0', sizeof(next->ospfNbrIp));
			memset(next->physAddress, '\0', sizeof(next->physAddress));

			pre->next = next;
			strcpy(next->ifdex, dp[0] ? dp[0] : "?");
			strcpy(next->ifDescr, dp[1] ? dp[1] : "?");
			strcpy(next->ifBandwidth, dp[4] ? dp[4] : "?");
			strcpy(next->ifAdminStatus, dp[6] ? dp[6] : "?");
			strcpy(next->ifOperStatus, dp[7] ? dp[7] : "?");
			strcpy(next->physAddress, dp[5] ? dp[5] : "?");
			if (strlen(next->ifDescr) > 19) {//huawei
				strncpy(next->ifDescrDex, next->ifDescr + 19, strlen(
						next->ifDescr) - 19 + 1);
			} else {//quagga
				strncpy(next->ifDescrDex, next->ifDescr + 3, strlen(
						next->ifDescr) - 3 + 1);
			}
			next->next = NULL;
			pre = next;
			dp = dp + fields;
		}
		free_data(data, entries, fields);
	}

	/**
	 * 获取IPADDRTABLE，将获取的内容相应的赋值给iptable结构体
	 */
	result = snmptable(routerIp, COMMUNITY, IPADDRTABLE, &entries, &fields,
			&data);
	if (result != 0) {
		printf("get IPADDRTABLE error\n");
	} else {
		dp = data;
		for (next = (*head)->next; next; next = next->next) {
			dp = data;
			for (entry = 0; entry < entries; entry++) {
				if (strcmp(next->ifdex, dp[1]) == 0) {
					strcpy(next->ip, dp[0] ? dp[0] : "?");
					strcpy(next->netMask, dp[2] ? dp[2] : "?");
					//				printf("[%i]:%s\n", entry + 1, dp[0] ? dp[0] : "?");
					break;
				}
				dp = dp + fields;
			}
		}
		free_data(data, entries, fields);
	}

	/**
	 * 获取BGPPEERTABLE，将邻居ip与当前路由器接口口ip比较在同一网段
	 */
	result = snmptable(routerIp, COMMUNITY, BGPPEERTABLE, &entries, &fields,
			&data);
	if (result != 0) {
		printf("get BGPPEERTABLE error\n");
	} else {
		for (next = (*head)->next; next; next = next->next) {
			dp = data;
			for (entry = 0; entry < entries; entry++) {
				if (next->ip != NULL && strcmp(next->ip, dp[4]) == 0) {
					strcpy(next->ospfNbrIp, dp[6] ? dp[6] : "?");//连接的peer ip
					strcpy(next->areaId, dp[8] ? dp[8] : "?");//对端的as-number
					strcpy(next->nbrRouteId, dp[0] ? dp[0] : "?");
					strcpy(next->ospfIfHelloInterval, dp[21] ? dp[21] : "?");//bgpPeerMinASOriginationInterval
					strcpy(next->ospfIfDeadInterval, dp[22] ? dp[22] : "?");//bgpPeerMinRouteAdvertisementInterval
					break;
				}
				dp = dp + fields;
			}
		}
		free_data(data, entries, fields);
	}
	printf("get ALL end\n");
	return 0;

}

/**
 * 获取路由器spf计算次数
 */
int getRouterSpfCount(char *routerIp, char *areaid) {
	//	puts(oids);
	int count = 0;
	int seq = 0;
	char spfCount[10];
	memset(spfCount, '\0', 10);
	char oids[100];
	sprintf(oids, "%s.%s", OSPFSPFRUNS, areaid);
	seq = snmpget(routerIp, COMMUNITY, oids, spfCount);
	if (seq < 0) {
		return -1;
	}
	count = stringToInt(spfCount);
	return count;
}
/**
 * 将字符串（中间含逗号），转为数字
 */
int stringToInt(char *string) {
	if (string == NULL) {
		return -1;
	}
	char *pre = string;
	char *next = string;
	char snum[100];
	int num = 0;
	memset(snum, '\0', 100);

	while (*next != '\0') {
		if (*next == ',') {
			strncat(snum, pre, num);
			pre = next + 1;
			num = 0;
		}
		next++;
		num++;
	}

	strncat(snum, pre, num);
	//	puts(snum);
	return atoi(snum);
}

long long stringTollInt(char *string) {
	if (string == NULL) {
		return -1;
	}
	char *pre = string;
	char *next = string;
	char snum[100];
	int num = 0;
	memset(snum, '\0', 100);

	while (*next != '\0') {
		if (*next == ',') {
			strncat(snum, pre, num);
			pre = next + 1;
			num = 0;
		}
		next++;
		num++;
	}

	strncat(snum, pre, num);
	//	puts(snum);
	return atoll(snum);
}

void sleeptime(int second) {
	fd_set rfds;
	struct timeval tv;
	int retval;
	/* Watch stdin (fd 0) to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	/* Wait up to five seconds. */
	tv.tv_sec = second;
	tv.tv_usec = 0;

	retval = select(1, NULL, NULL, NULL, &tv);

	/* Don't rely on the value of tv now! */

	if (retval == -1)

		perror("select()");

	else if (retval)
		printf("Data is available now.\n");
	/* FD_ISSET(0, &rfds) will be true. */
	else
		return ;

	return ;
}
