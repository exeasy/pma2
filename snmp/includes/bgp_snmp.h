/*
 * bgp_snmp.h
 *
 *  Created on: May 2, 2013
 *      Author: liupengzhan
 */

#ifndef BGP_SNMP_H_
#define BGP_SNMP_H_
#include "oids.h"
typedef struct bgpRouteTable{
	char bgp4PathAttrPeer[50];
	char bgp4PathAttrIpAddrPrefixLen[3];
	char bgp4PathAttrIpAddrPrefix[50];
	char bgp4PathAttrOrigin[10];
	char bgp4PathAttrASPathSegment[200];
	char bgp4PathAttrNextHop[50];
	char bgp4PathAttrMultiExitDisc[10];
	char bgp4PathAttrLocalPref[10];
	char bgp4PathAttrAtomicAggregate[50];
	char bgp4PathAttrAggregatorAS[10];
	char bgp4PathAttrAggregatorAddr[50];
	char bgp4PathAttrCalcLocalPref[10];
	char bgp4PathAttrBest[10];
	char bgp4PathAttrUnknown[10];
	struct bgpRouteTable *next;
}bgpRouteTable;

typedef struct bgpPeerTable{
	char bgpPeerLocalAddr[50];
	char bgpPeerRemoteAddr[50];
	char bgpPeerRemoteAs[50];
	char bgpPeerRemoteId[50];
	char metic[10];
	struct bgpPeerTable *next;
}bgpPeerTable;

#endif /* BGP_SNMP_H_ */
