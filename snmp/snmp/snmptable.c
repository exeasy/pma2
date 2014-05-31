/*
 * snmptable.c - walk a table and print it nicely
 *
 * Update: 1999-10-26 <rs-snmp@revelstone.com>
 * Added ability to use MIB to query tables with non-sequential column OIDs
 * Added code to handle sparse tables
 *
 * Update: 1998-07-17 <jhy@gsu.edu>
 * Added text <special options> to usage().
 */
/**********************************************************************
 Copyright 1997 Niels Baggesen

 All Rights Reserved

 Permission to use, copy, modify, and distribute this software and its
 documentation for any purpose and without fee is hereby granted,
 provided that the above copyright notice appear in all copies.

 I DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 I BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
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
# include <netinet/in.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
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
#include <stdio.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <pthread.h>
pthread_mutex_t snmp_lock = PTHREAD_MUTEX_INITIALIZER;

struct column {
	int width;
	oid subid;
	char *label;
	char *fmt;
}; //*column = NULL;

int snmptable(char *ip, char * comm, char *oidstr, int *entryptr,
		int *fieldptr, char ***dataptrr) {
	pthread_mutex_lock(&snmp_lock);
	struct column *column = NULL;
	char **data = NULL;
	char **indices = NULL;
	int index_width = sizeof("index ") - 1;
	int fields =0;
	int entries =0;
	int allocated=0;
	int end_of_table = 1;
	int headers_only = 0;
	int max_width = 0;
	int column_width = 0;
	int brief = 0;
	int show_index = 0;
	char *table_name = NULL;
	oid name[MAX_OID_LEN];
	size_t name_length;
	oid root[MAX_OID_LEN];
	size_t rootlen;
	int localdebug;
	int exitval = 0;
	int use_getbulk = 1;
	int max_getbulk = 10;

	int li = 0;
	for(li = 0 ; li<MAX_OID_LEN ; li++){
			root[li] = 0;
			name[li] = 0;
		}
	//****************************
	netsnmp_session session, *ss;
	int total_entries = 0;

	setvbuf(stdout, NULL, _IOLBF, 1024);
	netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT, 1);

	//<---------new-------->

	init_snmp("snmpd");
	snmp_sess_init(&session);
	session.version = SNMP_VERSION_2c;
	session.community = strdup(comm);
	session.community_len = (size_t) strlen(session.community);
	session.peername = strdup(ip);


	rootlen = MAX_OID_LEN;
	if (!snmp_parse_oid(oidstr, root, &rootlen)) {//查看mib树中是否有此节点
		snmp_perror(oidstr);
		return -1;
	}

	localdebug = netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
			NETSNMP_DS_LIB_DUMP_PACKET);
	int rs1 = 0;
	//*********************************
	//    rs1 = get_field_names();
	char *buf = NULL, *name_p = NULL;
	size_t buf_len = 0, out_len = 0;
#ifndef DISABLE_MIB_LOADING
	struct tree *tbl = NULL;
#endif /* DISABLE_MIB_LOADING */
	int going = 1;

#ifndef DISABLE_MIB_LOADING
	tbl = get_tree(root, rootlen, get_tree_head());
	//if (tbl) {
	//	tbl = tbl->child_list;
	//	if (tbl) {
	//		root[rootlen++] = tbl->subid;
	//		tbl = tbl->child_list;
	//	} else {
	//		root[rootlen++] = 1;
	//		going = 0;
	//	}
	//}
#endif /* DISABLE_MIB_LOotlen++] = tbl->subid;
                tbl = tbl->childADING */

	if (sprint_realloc_objid((u_char **) &buf, &buf_len, &out_len, 1, root,
			rootlen - 1)) {
		table_name = buf;
		buf = NULL;
		buf_len = out_len = 0;
	} else {
		table_name = strdup("[TRUNCATED]");
		out_len = 0;
	}

	fields = 0;
	while (going) {
		fields++;
		printf("XXXXXXXXXXXXXX\n");
#ifndef DISABLE_MIB_LOADING
		if (tbl) {
	//		if (tbl->access == MIB_ACCESS_NOACCESS) {
	//			printf("MIB_ACCESS_NOACCESS\n");
	//			fields--;
	//			tbl = tbl->next_peer;
	//			if (!tbl) {
	//				going = 0;
	//			}
	//			continue;
			}
			root[rootlen] = tbl->subid;
			tbl = tbl->next_peer;
			if (!tbl)
				going = 0;
		} else {
#endif /* DISABLE_MIB_LOADING */
			root[rootlen] = fields;
#ifndef DISABLE_MIB_LOADING
		}
#endif /* DISABLE_MIB_LOADING */
		out_len = 0;
		if (sprint_realloc_objid((u_char **) &buf, &buf_len, &out_len, 1, root,
				rootlen + 1)) {
			name_p = strrchr(buf, '.');
			if (name_p == NULL) {
				name_p = strrchr(buf, ':');
			}
			if (name_p == NULL) {
				name_p = buf;
			} else {
				name_p++;
			}
		} else {
			break;
		}
		if (localdebug) {
			printf("%s %c\n", buf, name_p[0]);
		}
		//if ('0' <= name_p[0] && name_p[0] <= '9') {
		//	fields--;
		//	break;
		//}
		if (fields == 1) {
			column = (struct column *) malloc(sizeof(*column));
		} else {
			column
					= (struct column *) realloc(column, fields
							* sizeof(*column));
		}
		column[fields - 1].label = strdup(name_p);
		column[fields - 1].width = strlen(name_p);
		column[fields - 1].subid = root[rootlen];
	}
	if (fields == 0) {
		fprintf(stderr, "Was that a table? %s\n", table_name);
		return 0;
	}
	if (name_p) {
		*name_p = 0;
		memmove(name, root, rootlen * sizeof(oid));
		name_length = rootlen + 1;
		name_p = strrchr(buf, '.');
		if (name_p == NULL) {
			name_p = strrchr(buf, ':');
		}
		if (name_p != NULL) {
			*name_p = 0;
		}
	}3
	if (brief && fields > 1) {
		char *f1, *f2;
		int common = strlen(column[0].label);
		int field, len;
		for (field = 1; field < fields; field++) {
			f1 = column[field - 1].label;
			f2 = column[field].label;
			while (*f1 && *f1++ == *f2++)
				;
			len = f2 - column[field].label - 1;
			if (len < common)
				common = len;
		}
		if (common) {
			for (field = 0; field < fields; field++) {
				column[field].label += common;
				column[field].width -= common;
			}
		}
	}
	if (buf != NULL) {
		free(buf);
	}
	rs1 = 1;

	// ***********************************
	if (rs1 == 0) {
		printf("\nget field name error!\n\n");
		return -3;
	}
	//*************
	//reverse_fields();
	struct column tmp;
	int i;

	for (i = 0; i < fields / 2; i++) {
		memcpy(&tmp, &(column[i]), sizeof(struct column));
		memcpy(&(column[i]), &(column[fields - 1 - i]), sizeof(struct column));
		memcpy(&(column[fields - 1 - i]), &tmp, sizeof(struct column));
	}
	//*******************************
	/*
	 * open an SNMP session
	 */
	SOCK_STARTUP;
	ss = snmp_sess_open(&session);
	if (ss == NULL) {
		/*
		 * diagnose snmp_open errors with the input netsnmp_session pointer
		 */
		snmp_sess_perror("snmptable", &session);
		SOCK_CLEANUP;
		return -2;
	}

#ifndef DISABLE_SNMPV1
	if (ss->version == SNMP_VERSION_1)
		use_getbulk = 1;
#endif
	//*********************
	int running = 1;
	netsnmp_pdu *pdu, *response;
	netsnmp_variable_list *vars, *last_var;
	int count;
	int status;
	i = 0;
	int row, col;
	char *tablebuf = NULL;
	size_t table_buf_len = 0;
	size_t table_out_len = 0;
	char *cp;
	char *table_name_p = NULL;
	char **dp;
	//***********************************
	do {
		entries = 0;
		allocated = 0;
		if (!headers_only) {
			if (use_getbulk) {
				//********************************************
				//				if (-1 == getbulk_table_entries(ss))
				//					printf("\ngetbulk table entries error!\n\n");


				//printf("getbulk_table_entries();\n");
				while (running) {
					/*
					 * create PDU for GETBULK request and add object name to request
					 */
					pdu = snmp_pdu_create(SNMP_MSG_GETBULK);
					pdu->non_repeaters = 0;
					pdu->max_repetitions = max_getbulk;
					snmp_add_null_var(pdu, name, name_length);

					/*
					 * do the request
					 */
					//add*****************
					if(ss == NULL){
						exitval = 1;
						break;
					}
//					puts("snmptablc.c snmp_synch_response~~~~~~~~~~~~~~~~~");
					status = snmp_sess_synch_response(ss, pdu, &response);
//					puts("snmptablc.c snmp_synch_response success~~~~~~~~~~~~~~~~~");
					if (status == STAT_SUCCESS) {
						//add***************************
						if (response == NULL) {
							printf("snmp response NULL\n");
							exitval = 1;
							break;
						}
						//****************************
						if (response->errstat == SNMP_ERR_NOERROR) {
							/*
							 * check resulting variables
							 */
							vars = response->variables;
							last_var = NULL;
							while (vars) {
								table_out_len = 0;
								sprint_realloc_objid((u_char **) &tablebuf,
										&table_buf_len, &table_out_len, 1,
										vars->name, vars->name_length);
								if (vars->type == SNMP_ENDOFMIBVIEW
										|| memcmp(vars->name, name, rootlen
												* sizeof(oid)) != 0) {
									if (localdebug) {
										printf("%s => end of table\n",
												tablebuf ? (char *) tablebuf
														: "[NIL]");
									}
									running = 0;
									break;
								}
								if (localdebug) {
									printf("%s => taken\n",
											tablebuf ? (char *) tablebuf
													: "[NIL]");
								}
								if (netsnmp_ds_get_boolean(
										NETSNMP_DS_LIBRARY_ID,
										NETSNMP_DS_LIB_EXTENDED_INDEX)) {
									table_name_p = strchr(tablebuf, '[');
								} else {
									switch (netsnmp_ds_get_int(
											NETSNMP_DS_LIBRARY_ID,
											NETSNMP_DS_LIB_OID_OUTPUT_FORMAT)) {
									case NETSNMP_OID_OUTPUT_MODULE:
									case 0:
										table_name_p = strrchr(tablebuf, ':');
										break;
									case NETSNMP_OID_OUTPUT_SUFFIX:
										table_name_p = tablebuf;
										break;
									case NETSNMP_OID_OUTPUT_FULL:
									case NETSNMP_OID_OUTPUT_NUMERIC:
									case NETSNMP_OID_OUTPUT_UCD:
										table_name_p = tablebuf + strlen(
												table_name) + 1;
										table_name_p
												= strchr(table_name_p, '.') + 1;
										break;
									default:
										fprintf(
												stderr,
												"Unrecognized -O option: %d\n",
												netsnmp_ds_get_int(
														NETSNMP_DS_LIBRARY_ID,
														NETSNMP_DS_LIB_OID_OUTPUT_FORMAT));
										return -1;
									}
									table_name_p = strchr(table_name_p, '.')
											+ 1;
								}
								for (row = 0; row < entries; row++)
									if (strcmp(table_name_p, indices[row]) == 0)
										//                    	if (strncmp(name_p, indices[row], strlen(indices[row])) == 0)
										break;
								if (row == entries) {
									entries++;
									if (entries >= allocated) {
										if (allocated == 0) {
											allocated = 10;
											data = (char **) malloc(allocated
													* fields * sizeof(char *));
											memset(data, 0, allocated * fields
													* sizeof(char *));
											indices = (char **) malloc(
													allocated * sizeof(char *));
										} else {
											allocated += 10;
											data = (char **) realloc(data,
													allocated * fields
															* sizeof(char *));
											memset(data + entries * fields, 0,
													(allocated - entries)
															* fields
															* sizeof(char *));
											indices = (char **) realloc(
													indices, allocated
															* sizeof(char *));
										}
									}
									indices[row] = strdup(table_name_p);
									i = strlen(table_name_p);
									if (i > index_width)
										index_width = i;
								}
								dp = data + row * fields;
								table_out_len = 0;
								sprint_realloc_value((u_char **) &tablebuf,
										&table_buf_len, &table_out_len, 1,
										vars->name, vars->name_length, vars);
								for (cp = tablebuf; *cp; cp++)
									if (*cp == '\n')
										*cp = ' ';
								for (col = 0; col < fields; col++)
									if (column[col].subid
											== vars->name[rootlen])
										break;
								dp[col] = tablebuf;
								i = table_out_len;
//								if(tablebuf != NULL){
//									free(tablebuf);
//								}
								tablebuf = NULL;
								table_buf_len = 0;
								if (i > column[col].width)
									column[col].width = i;
								last_var = vars;
								vars = vars->next_variable;
							}
							if (last_var) {
								name_length = last_var->name_length;
								memcpy(name, last_var->name, name_length
										* sizeof(oid));
							}
						} else {
							/*
							 * error in response, print it
							 */
							running = 0;
							if (response->errstat == SNMP_ERR_NOSUCHNAME) {
								printf("End of MIB\n");
							} else {
								fprintf(stderr,
										"Error in packet.\nReason: %s\n",
										snmp_errstring(response->errstat));
								if (response->errstat == SNMP_ERR_NOSUCHNAME) {
									fprintf(stderr,
											"The request for this object identifier failed: ");
									for (count = 1, vars = response->variables; vars
											&& count != response->errindex; vars
											= vars->next_variable, count++)
										/*EMPTY*/;
									if (vars) {
										fprint_objid(stderr, vars->name,
												vars->name_length);
									}
									fprintf(stderr, "\n");
								}
								exitval = 2;
							}
						}
					} else if (status == STAT_TIMEOUT) {
						fprintf(stderr, "Timeout: No Response from %s\n",
								ss->peername);
						running = 0;
						exitval = 1;
					} else { /* status == STAT_ERROR */
						snmp_sess_perror("snmptable", ss);
						running = 0;
						exitval = 1;
					}
					if (response)
						snmp_free_pdu(response);
				}
//				return 1;
			}

			//***************************************************
//			else if (-1 == get_table_entries(ss)) //v1
			else
				printf("\nget table entries error!\n\n");
		}

		if (exitval) {
//			free(session.community);
//			free(session.peername);
			snmp_sess_close(ss);
			SOCK_CLEANUP;
			return exitval;
		}

		//        if (entries || headers_only)
		//            print_table0();

		/*
		 if (data) {
		 free (data);
		 data = NULL;
		 }
		 */
		//free indices
		for(i = 0; i < entries; i ++){
			free(indices[i]);
		}
		if (indices) {
			free(indices);
			indices = NULL;
		}
		//

		total_entries += entries;

	} while (!end_of_table);

	//free column
	for(i = 0 ;i <fields;i++){
		if (column[i].label != NULL) {
			free(column[i].label);
		}

	}

	if(column != NULL){
		free(column);
		column = NULL;
	}
	//

//	free(session.community);
//	free(session.peername);

	snmp_sess_close(ss);

	SOCK_CLEANUP;

	if (total_entries == 0) {
		printf("%s: No entries\n", table_name);
		return 1;
	}

	*fieldptr = fields;
	*entryptr = entries;
	*dataptrr = data;
	//    memcpy(&dataptr,&data,sizeof(&data));
//	snmp_close(&session);

	pthread_mutex_unlock(&snmp_lock);
	return 0;
}
void free_data(char **datap, int e, int f) {
	int i, j;
	char **dp = datap;
	for (j = 0; j < e; j++) {
		for (i = 0; i < f; i++) {
			if (dp[i])
				free(dp[i]);
		}
		dp += f;
	}
	if (datap)
		free(datap);
	datap = NULL;
}


void print_table(char **data, int entries, int fields) {
	int entry, field;
	char **dp;

	//	printf("SNMP table: %s\n\n", table_name);
	printf("entry: %i\n", entries);
	printf("field: %i\n", fields);

	dp = data;
	for (entry = 0; entry < entries; entry++) {
		for (field = 0; field < fields; field++) {
			printf("[%i][%i]:%s\n", entry + 1, field + 1, dp[field] ? dp[field]
					: "?");
			//printf("%x\n",dp[field]);
		}
		dp += fields;
		printf("\n");
	}
}

