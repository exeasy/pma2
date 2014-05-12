/*
 * snmptrap.c
 *
 *  Created on: Dec 14, 2012
 *      Author: yx
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>
#include <stdio.h>

oid             objid_enterprise[] = { 1, 3, 6, 1, 4, 1, 3, 1, 1 };
oid             objid_sysdescr[] = { 1, 3, 6, 1, 2, 1, 1, 1, 0 };
oid             objid_sysuptime[] = { 1, 3, 6, 1, 2, 1, 1, 3, 0 };
oid             objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
//int snmp_input(int operation, netsnmp_session * session, int reqid,
//		netsnmp_pdu *pdu, void *magic) {
//	return 1;
//}

int snmptrap2(char *ip, char *oidNO1, char *oidNO2, char type, char *text) {
	if (ip == NULL || strlen(ip) == 0) {
		fprintf(stderr, "No ip\n");
		return (1);
	}
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	oid anOID[MAX_OID_LEN];
	long sysuptime;
	char csysuptime[20];
	int status = 0;
	oid oid_sysuptime[] = { 1, 3, 6, 1, 2, 1, 1, 3, 0 };
	char *community = "public";
	netsnmp_transport *transport = NULL;
	size_t anOID_len = MAX_OID_LEN;

	snmp_sess_init(&session);
	session.version = SNMP_VERSION_2c;
	session.peername = ip;
	session.remote_port = 162;
	session.community = (unsigned char*) community;
	session.community_len = strlen((char *) session.community);
	session.retries = 3;
	session.timeout = 2000;
	session.sessid = 0;

	SOCK_STARTUP;

	transport = netsnmp_transport_open_client("snmptrap", session.peername);
	if (NULL != transport)
		ss = snmp_add(&session, transport, NULL, NULL);

	if (ss == NULL) {
		snmp_sess_perror("snmptable", &session);
		SOCK_CLEANUP;
	}

	pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
	sysuptime = get_uptime();
	sprintf(csysuptime, "%ld", sysuptime);

	status = snmp_add_var(pdu, objid_sysuptime,
			sizeof(objid_sysuptime) / sizeof(oid), 't', csysuptime);
	if (status != 0) {
		snmp_sess_perror("snmptrap add sysuptime error!", &session);
		return -1;
	}
	if (snmp_add_var
	            (pdu, objid_snmptrap, sizeof(objid_snmptrap) / sizeof(oid),
	             'o', oidNO1) != 0) {
	            snmp_perror(oidNO1);
	            SOCK_CLEANUP;
	            exit(1);
	        }

	if (!snmp_parse_oid(oidNO2, anOID, &anOID_len)) {
		  snmp_perror(oidNO2);
		  SOCK_CLEANUP;
		  exit(1);
		}
	if(snmp_add_var(pdu, anOID, anOID_len, type,
			text)!= 0) {
        snmp_perror(text);
        SOCK_CLEANUP;
        exit(1);
    }
	status = snmp_send(ss, pdu);
	if (status == 0) {
		snmp_sess_perror("snmptrap send info error!", &session);
		snmp_free_pdu(pdu);
		return -2;
	}
	snmp_close(ss);
	snmp_shutdown("snmpapp");
	SOCK_CLEANUP;
	printf(stderr, "send snmptrap over %d... ", status);
	return 0;
}

