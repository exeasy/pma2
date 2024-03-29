/*
 * snmpset.c - send snmp SET requests to a network entity.
 *
 */
/***********************************************************************
	Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <stdio.h>
#include <ctype.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <net-snmp/net-snmp-includes.h>

//void
//usage(void)
//{
//    fprintf(stderr, "USAGE: snmpset ");
//    snmp_parse_args_usage(stderr);
//    fprintf(stderr, " OID TYPE VALUE [OID TYPE VALUE]...\n\n");
//    snmp_parse_args_descriptions(stderr);
//    fprintf(stderr,
//            "  -C APPOPTS\t\tSet various application specific behaviours:\n");
//    fprintf(stderr, "\t\t\t  q:  don't print results on success\n");
//    fprintf(stderr, "\n  TYPE: one of i, u, t, a, o, s, x, d, b\n");
//    fprintf(stderr,
//            "\ti: INTEGER, u: unsigned INTEGER, t: TIMETICKS, a: IPADDRESS\n");
//    fprintf(stderr,
//            "\to: OBJID, s: STRING, x: HEX STRING, d: DECIMAL STRING, b: BITS\n");
//#ifdef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
//    fprintf(stderr,
//            "\tU: unsigned int64, I: signed int64, F: float, D: double\n");
//#endif                          /* NETSNMP_WITH_OPAQUE_SPECIAL_TYPES */
//
//}

static int quiet = 0;

static
    void
optProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'q':
                quiet = 1;
                break;

            default:
                fprintf(stderr, "Unknown flag passed to -C: %c\n",
                        optarg[-1]);
                exit(1);
            }
        }
    }
}

int
snmpset(char *ip,char *vision,char* commmunity,char *oid_array,char type,char *value)
{
	 int set_flag = 1;
	    netsnmp_session session, *ss;
	    netsnmp_pdu    *pdu, *response = NULL;
	    netsnmp_variable_list *vars;
	    int             arg;
	    int             count;
	    int             current_name = 0;
	    int             current_type = 0;
	    int             current_value = 0;
	    char           *names[SNMP_MAX_CMDLINE_OIDS];
	    char            types[SNMP_MAX_CMDLINE_OIDS];
	    char           *values[SNMP_MAX_CMDLINE_OIDS];
	    oid             name[MAX_OID_LEN];
	    size_t          name_length;
	    int             status;
	    int             failures = 0;
	    int             exitval = 0;

	    putenv(strdup("POSIXLY_CORRECT=1"));

	    /*
	     * get the common command line arguments
	     */
	    snmp_sess_init(&session);

	    if (!strcmp(vision, "1")) {
	         session.version = SNMP_VERSION_1;
	     }

	     if (!strcasecmp(vision, "2c")) {
	         session.version = SNMP_VERSION_2c;
	     }

	     if (!strcasecmp(vision, "3")) {
	         session.version = SNMP_VERSION_3;
	     }
	     session.peername = ip;
	     session.community = (unsigned char*)commmunity;
	     session.community_len = strlen(commmunity);


	     names[0] = oid_array;
	     current_name = 1;
	     types[0] = type;
	     current_type = 1;
	     values[0] = value;
	     current_value = 1;

	    SOCK_STARTUP;

	    /*
	     * open an SNMP session
	     */
	    snmp_close(ss);
	    ss = snmp_open(&session);
	    if (ss == NULL) {
	        /*
	         * diagnose snmp_open errors with the input netsnmp_session pointer
	         */
	        snmp_sess_perror("snmpset", &session);
	        SOCK_CLEANUP;
	        set_flag = 0;
	        return set_flag;
	    }

	    /*
	     * create PDU for SET request and add object names and values to request
	     */
	    pdu = snmp_pdu_create(SNMP_MSG_SET);
	    for (count = 0; count < current_name; count++) {
	        name_length = MAX_OID_LEN;
	        if (snmp_parse_oid(names[count], name, &name_length) == NULL) {
	            snmp_perror(names[count]);
	            failures++;
	        } else
	            if (snmp_add_var
	                (pdu, name, name_length, types[count], values[count])) {
	            snmp_perror(names[count]);
	            failures++;
	        }
	    }

	    if (failures) {
	        snmp_close(ss);
	        SOCK_CLEANUP;
	        set_flag = 0;
	        return set_flag;
	    }

	    /*
	     * do the request
	     */
	    status = snmp_synch_response(ss, pdu, &response);
	    if (status == STAT_SUCCESS) {
	        if (response->errstat == SNMP_ERR_NOERROR) {
	            if (!quiet) {
	                for (vars = response->variables; vars;
	                     vars = vars->next_variable)
	                    print_variable(vars->name, vars->name_length, vars);
	            }
	        } else {
	            fprintf(stderr, "Error in packet.\nReason: %s\n",
	                    snmp_errstring(response->errstat));
	            if (response->errindex != 0) {
	                fprintf(stderr, "Failed object: ");
	                for (count = 1, vars = response->variables;
	                     vars && (count != response->errindex);
	                     vars = vars->next_variable, count++);
	                if (vars)
	                    fprint_objid(stderr, vars->name, vars->name_length);
	                fprintf(stderr, "\n");
	            }
	            exitval = 2;
	        }
	    } else if (status == STAT_TIMEOUT) {
	        fprintf(stderr, "Timeout: No Response from %s\n",
	                session.peername);
	        exitval = 1;
	    } else {                    /* status == STAT_ERROR */
	        snmp_sess_perror("snmpset", ss);
	        exitval = 1;
	    }

	    if (response)
	        snmp_free_pdu(response);
	    snmp_close(ss);
	    SOCK_CLEANUP;
	    set_flag = 0;
	    return exitval;
}
