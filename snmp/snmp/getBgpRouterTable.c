/*
 * getBgpRouteTable.c
 *
 *  Created on: May 2, 2013
 *      Author: liupengzhan
 */

#include "oids.h"
#include "bgp_snmp.h"
#include  <stdio.h>

void printBgpRouteTable(bgpRouteTable *head) {
	bgpRouteTable *it = head;
	while (it) {
		printf("bgp4PathAttrPeer = %s\n", it->bgp4PathAttrPeer);
		printf("bgp4PathAttrIpAddrPrefixLen = %s \n", it->bgp4PathAttrIpAddrPrefixLen);
		printf("bgp4PathAttrIpAddrPrefix = %s\n", it->bgp4PathAttrIpAddrPrefix);
		printf("bgp4PathAttrOrigin = %s\n", it->bgp4PathAttrOrigin);
		printf("bgp4PathAttrASPathSegment = %s\n", it->bgp4PathAttrASPathSegment);
		printf("bgp4PathAttrNextHop = %s\n", it->bgp4PathAttrNextHop);
		printf("bgp4PathAttrMultiExitDisc = %s\n", it->bgp4PathAttrMultiExitDisc);
		it = it->next;
		printf("\n");
	}
}

void freeBgpRouteTable(bgpRouteTable *head) {
	bgpRouteTable *ita = head;
	bgpRouteTable *itb = NULL;
	while (ita) {
		itb = ita->next;
		free(ita);
		ita = itb;
	}
}

int getBgpRouteTable(char *routerIp, bgpRouteTable **head) {
	if (routerIp == NULL) {
		return -1;
	}
	int fields, entries, result;
	char ** data = NULL;
	char ** dp = NULL;
	bgpRouteTable *pre, *next;//*head,
	int entry, field;

	next = (bgpRouteTable *) malloc(sizeof(bgpRouteTable));
	*head = next;
	pre = next;
	(*head)->next = NULL;

	result = snmptable(routerIp, COMMUNITY, BGP4PATHATTRTABLE, &entries,
			&fields, &data);

	if (result != 0) {
		printf("get BGP4PATHATTRTABLE error\n");
	}
	dp = data;
	for (entry = 0; entry < entries; entry++) {
		next = (bgpRouteTable *) malloc(sizeof(bgpRouteTable));

		memset(next, '\0', sizeof(bgpRouteTable));

		pre->next = next;
		strcpy(next->bgp4PathAttrPeer, dp[0] ? dp[0] : "?");
		strcpy(next->bgp4PathAttrIpAddrPrefixLen, dp[1] ? dp[1] : "?");
		strcpy(next->bgp4PathAttrIpAddrPrefix, dp[2] ? dp[2] : "?");
		strcpy(next->bgp4PathAttrOrigin, dp[3] ? dp[3] : "?");
		strcpy(next->bgp4PathAttrASPathSegment, dp[4] ? dp[4] : "?");
		strcpy(next->bgp4PathAttrNextHop, dp[5] ? dp[5] : "?");
		strcpy(next->bgp4PathAttrMultiExitDisc, dp[6] ? dp[6] : "?");
		strcpy(next->bgp4PathAttrLocalPref, dp[7] ? dp[7] : "?");
		strcpy(next->bgp4PathAttrAtomicAggregate, dp[8] ? dp[8] : "?");
		strcpy(next->bgp4PathAttrAggregatorAS, dp[9] ? dp[9] : "?");
		strcpy(next->bgp4PathAttrAggregatorAddr, dp[10] ? dp[10] : "?");
		strcpy(next->bgp4PathAttrCalcLocalPref, dp[11] ? dp[11] : "?");
		strcpy(next->bgp4PathAttrBest, dp[12] ? dp[12] : "?");
		strcpy(next->bgp4PathAttrUnknown, dp[13] ? dp[13] : "?");
		next->next = NULL;
		pre = next;
		dp = dp + fields;
	}
	free_data(data, entries, fields);
	return 0;
}

void printBgpPeerTable(bgpPeerTable *head) {
	bgpPeerTable *it = head;
	while (it) {
		printf("bgpPeerLocalAddr = %s\n", it->bgpPeerLocalAddr);
		printf("bgpPeerRemoteAddr = %s \n", it->bgpPeerRemoteAddr);
		printf("bgpPeerRemoteAs = %s\n", it->bgpPeerRemoteAs);
		printf("bgpPeerRemoteId = %s\n", it->bgpPeerRemoteId);
		printf("metic = %s\n", it->metic);
		it = it->next;
		printf("\n");
	}
}

void freeBgpPeerTable(bgpPeerTable *head) {
	bgpPeerTable *ita = head;
	bgpPeerTable *itb = NULL;
	while (ita) {
		itb = ita->next;
		free(ita);
		ita = itb;
	}
}

int getBgpPeerTable(char *routerIp, bgpPeerTable **head) {
	if (routerIp == NULL) {
		return -1;
	}
	int fields, entries, result;
	char ** data = NULL;
	char ** dp = NULL;
	bgpPeerTable *pre, *next;//*head,
	int entry, field;

	next = (bgpPeerTable *) malloc(sizeof(bgpPeerTable));
	*head = next;
	pre = next;
	(*head)->next = NULL;

	result = snmptable(routerIp, COMMUNITY, BGPPEERTABLE, &entries, &fields,
			&data);

	if (result != 0) {
		printf("get BGPPEERTABLE error\n");
	}
	dp = data;
	for (entry = 0; entry < entries; entry++) {
		next = (bgpPeerTable *) malloc(sizeof(bgpPeerTable));

		memset(next->bgpPeerLocalAddr, '\0', sizeof(next->bgpPeerLocalAddr));
		memset(next->bgpPeerRemoteAddr, '\0', sizeof(next->bgpPeerRemoteAddr));
		memset(next->bgpPeerRemoteAs, '\0', sizeof(next->bgpPeerRemoteAs));
		memset(next->bgpPeerRemoteId, '\0', sizeof(next->bgpPeerRemoteId));
		memset(next->metic, '\0', sizeof(next->metic));

		pre->next = next;
		strcpy(next->bgpPeerRemoteId, dp[0] ? dp[0] : "?");
		strcpy(next->bgpPeerLocalAddr, dp[4] ? dp[4] : "?");
		strcpy(next->bgpPeerRemoteAddr, dp[6] ? dp[6] : "?");
		strcpy(next->bgpPeerRemoteAs, dp[8] ? dp[8] : "?");
		next->next = NULL;
		pre = next;
		dp = dp + fields;
	}
	free_data(data, entries, fields);

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
				if (strcmp(next->bgpPeerLocalAddr, dp[0]) == 0) {
					strcpy(next->metic, dp[3] ? dp[3] : "?");
					//					printf("[%i]:%s\n", entry + 1, dp[0] ? dp[0] : "?");
					break;
				}
			}
			dp = dp + fields;
		}
		free_data(data, entries, fields);
	}
	puts("Get bgp peer table end!");
	return 0;
}

int ipRouteToSring(char *string, ipRouteTable *head){
	if (string == NULL || head == NULL) {
		return -1;
	}
	ipRouteTable * node = head;
	sprintf(
			string + strlen(string),
			"%s\n",
			"ipRouteDest ipRouteMask ipRouteNextHop ipRouteIfIndex ipRouteType ipRouteProto ipRouteAge ipRouteMetric");
	while (node) {
		sprintf(string + strlen(string), "%s ", node->ipRouteDest);
		sprintf(string + strlen(string), "%s ", node->ipRouteMask);
		sprintf(string + strlen(string), "%s ", node->ipRouteNextHop);
		sprintf(string + strlen(string), "%s ", node->ipRouteIfIndex);
		sprintf(string + strlen(string), "%s ", node->ipRouteType);
		sprintf(string + strlen(string), "%s ", node->ipRouteProto);
		sprintf(string + strlen(string), "%s ", node->ipRouteAge);
		sprintf(string + strlen(string), "%s\n", node->ipRouteMetric);

		node = node->next;
	}
}
int bgpRouteToSring(char *string, bgpRouteTable *head){
	if (string == NULL || head == NULL) {
		return -1;
	}
	bgpRouteTable * node = head;
	sprintf(
			string + strlen(string),
			"%s\n",
			"bgp4PathAttrPeer bgp4PathAttrIpAddrPrefixLen bgp4PathAttrIpAddrPrefix bgp4PathAttrOrigin bgp4PathAttrASPathSegment bgp4PathAttrNextHop bgp4PathAttrMultiExitDisc bgp4PathAttrLocalPref bgp4PathAttrAtomicAggregate bgp4PathAttrAggregatorAS bgp4PathAttrAggregatorAddr bgp4PathAttrCalcLocalPref bgp4PathAttrBest bgp4PathAttrUnknown");
	while (node) {
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrPeer);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrIpAddrPrefixLen);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrIpAddrPrefix);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrOrigin);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrASPathSegment);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrNextHop);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrMultiExitDisc);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrLocalPref);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrAtomicAggregate);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrAggregatorAS);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrAggregatorAddr);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrCalcLocalPref);
		sprintf(string + strlen(string), "%s ", node->bgp4PathAttrBest);
		sprintf(string + strlen(string), "%s\n", node->bgp4PathAttrUnknown);
		node = node->next;
	}
}
