#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oids.h"

//apt-get install expect-dev
//apt-get install tcl libtcl expect
/**
 * 向路由器添加一条静态路由
 */
int addStaticRoute(char * routerIp, char * desIp, char *mask, char * nextHop){
	if(routerIp == NULL || desIp == NULL || mask == NULL || nextHop == NULL){
		return -1;
	}
	int resault = 0;
	char routerType[MAX_LEN];
	char command[MAX_LEN];
	memset(routerType, '\0', sizeof(routerType));
	memset(command,'\0', sizeof(command));
	snmpget(routerIp, COMMUNITY, SYSTEMNAME, routerType);
	if (strcmp(routerType, HUAWEI_SYSTEMNAME) == 0) {
		sprintf(command,"expect %s%s %s %s %s %s", FILEPATH, FILENAME_ADDSTATICROUTE_HUAWEI, routerIp, desIp, mask, nextHop);
		resault = system(command);
	} else {
		sprintf(command,"expect %s%s %s %s %s %s", FILEPATH, FILENAME_ADDSTATICROUTE_QUAGGA, routerIp, desIp, mask, nextHop);
		resault = system(command);
	}
	return resault;
}
/**
 *  删除静态路由
 */
int removeStaticRoute(char * routerIp, char * desIp, char *mask, char * nextHop){
	if(routerIp == NULL || desIp == NULL || mask == NULL || nextHop == NULL){
			return -1;
	}
	int resault = 0;
	char routerType[MAX_LEN];
	char command[MAX_LEN];
	memset(routerType, '\0', sizeof(routerType));
	memset(command,'\0', sizeof(command));
	snmpget(routerIp, COMMUNITY, SYSTEMNAME, routerType);
	if (strcmp(routerType, HUAWEI_SYSTEMNAME) == 0) {
		sprintf(command,"expect %s%s %s %s %s %s", FILEPATH, FILENAME_REMOVESTATICROUTE_HUAWEI, routerIp, desIp, mask, nextHop);
		resault = system(command);
	} else {
		sprintf(command,"expect %s%s %s %s %s %s", FILEPATH, FILENAME_REMOVESTATICROUTE_QUAGGA, routerIp, desIp, mask, nextHop);
		resault = system(command);
	}
	return resault;
}

/**
 * 设置路由器端口的hello定时器值，成功返回0.
 */
int setHelloInteval(char * routerIp, char *ethIdNum, char *value){
	if(routerIp == NULL || ethIdNum == NULL || value == NULL ){
		return -1;
	}
	int resault = 0;
	char routerType[MAX_LEN];
	char command[MAX_LEN];
	memset(routerType, '\0', sizeof(routerType));
	memset(command, '\0', sizeof(command));
	snmpget(routerIp, COMMUNITY, SYSTEMNAME, routerType);
	if (strcmp(routerType, HUAWEI_SYSTEMNAME) == 0) {
		if (!strstr(ethIdNum, "GigabitEthernet") == NULL) {
			sprintf(command, "expect %s%s %s %s %s", FILEPATH,
					FILENAME_SETHELLO_HUAWEI, routerIp, ethIdNum, value);
		} else {
			sprintf(command, "expect %s%s %s GigabitEthernet0/0/%s %s",
					FILEPATH, FILENAME_SETHELLO_HUAWEI, routerIp, ethIdNum,
					value);
		}
		printf("%s\n", command);
	} else {
		if (!strstr(ethIdNum, "eth") == NULL) {
			sprintf(command, "expect %s%s %s %s %s", FILEPATH,
					FILENAME_SETHELLO_QUAGGA, routerIp, ethIdNum, value);
		} else {
			sprintf(command, "expect %s%s %s eth%s %s", FILEPATH,
					FILENAME_SETHELLO_QUAGGA, routerIp, ethIdNum, value);
		}
	}
	resault = system(command);
	return resault;
}

/**
 * 设置路由器端口的dead定时器值，成功返回0.
 */
int setDeadInteval(char * routerIp, char *ethIdNum, char *value){
	if (routerIp == NULL || ethIdNum == NULL || value == NULL) {
		return -1;
	}
	int resault = 0;
	char routerType[MAX_LEN];
	char command[MAX_LEN];
	memset(routerType, '\0', sizeof(routerType));
	memset(command, '\0', sizeof(command));
	snmpget(routerIp, COMMUNITY, SYSTEMNAME, routerType);
	if (strcmp(routerType, HUAWEI_SYSTEMNAME) == 0) {
		if (!strstr(ethIdNum, "GigabitEthernet") == NULL) {
			sprintf(command, "expect %s%s %s %s %s", FILEPATH,
					FILENAME_SETDEAD_HUAWEI, routerIp, ethIdNum, value);
		} else {
			sprintf(command, "expect %s%s %s GigabitEthernet0/0/%s %s", FILEPATH,
					FILENAME_SETDEAD_HUAWEI, routerIp, ethIdNum, value);
		}
	} else {
		if (!strstr(ethIdNum, "eth") == NULL) {
			sprintf(command, "expect %s%s %s %s %s", FILEPATH,
					FILENAME_SETDEAD_QUAGGA, routerIp, ethIdNum, value);
		} else {
			sprintf(command, "expect %s%s %s eth%s %s", FILEPATH,
					FILENAME_SETDEAD_QUAGGA, routerIp, ethIdNum, value);
		}

	}
	resault = system(command);
	return resault;
}

/**
 * 设置路由器端口的cost值，成功返回0.
 */
int setCost(char * routerIp, char *ethIdNum, char *value){
	if (routerIp == NULL || ethIdNum == NULL || value == NULL) {
		return -1;
	}
	int resault = 0;
	char routerType[MAX_LEN];
	char command[MAX_LEN];
	memset(routerType, '\0', sizeof(routerType));
	memset(command, '\0', sizeof(command));
	snmpget(routerIp, COMMUNITY, SYSTEMNAME, routerType);
	if (strcmp(routerType, HUAWEI_SYSTEMNAME) == 0) {
		if (!strstr(ethIdNum, "GigabitEthernet") == NULL) {
			sprintf(command, "expect %s%s %s %s %s", FILEPATH,
					FILENAME_SETCOST_HUAWEI, routerIp, ethIdNum, value);
		} else {
			sprintf(command, "expect %s%s %s GigabitEthernet0/0/%s %s", FILEPATH,
					FILENAME_SETCOST_HUAWEI, routerIp, ethIdNum, value);
		}
	} else {
		if (!strstr(ethIdNum, "eth") == NULL) {
			sprintf(command, "expect %s%s %s %s %s", FILEPATH,
					FILENAME_SETCOST_QUAGGA, routerIp, ethIdNum, value);
		} else {
			sprintf(command, "expect %s%s %s eth%s %s", FILEPATH,
					FILENAME_SETCOST_QUAGGA, routerIp, ethIdNum, value);
		}
	}
	resault = system(command);
	return resault;
}

/**
 * 设置路由器端口的Retransmit定时器值，成功返回0.
 */
int setRetransmitInteval(char * routerIp, char *ethIdNum, char *value){
	if (routerIp == NULL || ethIdNum == NULL || value == NULL) {
		return -1;
	}
	int resault = 0;
	char routerType[MAX_LEN];
	char command[MAX_LEN];
	memset(routerType, '\0', sizeof(routerType));
	memset(command, '\0', sizeof(command));
	snmpget(routerIp, COMMUNITY, SYSTEMNAME, routerType);
	if (strcmp(routerType, HUAWEI_SYSTEMNAME) == 0) {
		if (!strstr(ethIdNum, "GigabitEthernet") == NULL) {
			sprintf(command, "expect %s%s %s %s %s", FILEPATH,
					FILENAME_SETRETRANSMIT_HUAWEI, routerIp, ethIdNum, value);
		} else {
			sprintf(command, "expect %s%s %s GigabitEthernet0/0/%s %s", FILEPATH,
					FILENAME_SETRETRANSMIT_HUAWEI, routerIp, ethIdNum, value);
		}
	} else {
		if (!strstr(ethIdNum, "eth") == NULL) {
			sprintf(command, "expect %s%s %s %s %s", FILEPATH,
					FILENAME_SETRETRANSMIT_QUAGGA, routerIp, ethIdNum, value);
		} else {
			sprintf(command, "expect %s%s %s eth%s %s", FILEPATH,
					FILENAME_SETRETRANSMIT_QUAGGA, routerIp, ethIdNum, value);
		}

	}
	resault = system(command);
	return resault;
}

int setBgpPeerUpdateInterval(char *routerIp, char *bgpAreaId, char *peerIp,
		int interval) {
	if (routerIp == NULL || bgpAreaId == NULL || peerIp == NULL) {
		return -1;
	}
	int resault = 0;
	char routerType[MAX_LEN];
	char command[MAX_LEN];
	memset(routerType, '\0', sizeof(routerType));
	memset(command, '\0', sizeof(command));
	snmpget(routerIp, COMMUNITY, SYSTEMNAME, routerType);
	if (strcmp(routerType, HUAWEI_SYSTEMNAME) == 0) {
		sprintf(command, "expect %s%s %s %s %s %d", FILEPATH,
				FILENAME_SETBGPUPTADEINTERVAL_HUAWEI, routerIp, bgpAreaId,
				peerIp, interval);
	} else {
		sprintf(command, "expect %s%s %s %s %s %d", FILEPATH,
				FILENAME_SETBGPUPTADEINTERVAL_QUAGGA, routerIp, bgpAreaId,
				peerIp, interval);
	}
	resault = system(command);
	return resault;
}
