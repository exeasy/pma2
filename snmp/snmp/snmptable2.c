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
#include <stdio.h>
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <includes/snmpget.h>
#include <pthread.h>
pthread_mutex_t snmp_lock = PTHREAD_MUTEX_INITIALIZER;

struct column {
    int             width;
    oid             subid;
    char           *label;
    char           *fmt;
} *column = NULL;     

static char   **data = NULL;
static char   **indices = NULL;
static int      index_width = sizeof("index ") - 1;
static int      fields;
static int      entries;
static int      allocated;
static int      end_of_table = 1;
static int      headers_only = 0;
static int      no_headers = 0;
static int      max_width = 0;
static int      column_width = 0;
static int      brief = 0;
static int      show_index = 0;
static const char    *left_justify_flag = "";
static char    *field_separator = NULL;
static char    *table_name;
static oid      name[MAX_OID_LEN];
static size_t   name_length;
static oid      root[MAX_OID_LEN];
static size_t   rootlen;
static int      localdebug=1;
static int      exitval = 0;
static int      use_getbulk = 1;
static int      max_getbulk = 10;
static int      extra_columns = 0;

void            usage(void);
void            get_field_names(void);
void            get_table_entries(netsnmp_session * ss);
//void            getbulk_table_entries(netsnmp_session * ss);
void getbulk_table_entries(netsnmp_session * ss,int* entryptr, int* fieldptr, char*** datastr);
//void			getbulk_table_entries(netsnmp_session * ss, char*** datastr);
void            print_table(void);


void
reverse_fields(void)
{
    struct column   tmp;
    int             i;

    for (i = 0; i < fields / 2; i++) {
        memcpy(&tmp, &(column[i]), sizeof(struct column));
        memcpy(&(column[i]), &(column[fields - 1 - i]),
               sizeof(struct column));
        memcpy(&(column[fields - 1 - i]), &tmp, sizeof(struct column));
    }
}

int
snmptable(char *ip ,char *comm, char *oid, int* entryptr, 
		int *fieldptr, char ***datastr)
{
	pthread_mutex_lock(&snmp_lock);
	INIT_CLEANER
    netsnmp_session session, *ss;
    int            total_entries = 0;

//    netsnmp_set_line_buffering(stdout);
    setvbuf(stdout, NULL, _IOLBF, 1024);
    netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID,
                           NETSNMP_DS_LIB_QUICK_PRINT, 1);
	column = NULL;
	data = NULL;
	indices = NULL;
	fields =0;
	entries =0;
	allocated=0;
	end_of_table = 1;
	headers_only = 0;
	no_headers = 0;
	max_width = 0;
	column_width = 0;
	brief = 0;
	show_index = 0;
	table_name = NULL;
	memset(name,0, MAX_OID_LEN* sizeof(oid));
	name_length = 0;
	memset(root,0,MAX_OID_LEN* sizeof(oid));
	exitval = 0;
	use_getbulk = 1;
	max_getbulk = 10;
	extra_columns= 0;


        init_snmp("snmpd");
        snmp_sess_init( &session );
        session.version = SNMP_VERSION_2c;
        session.community = Z_strdup(comm);
        session.community_len = (size_t)strlen(session.community);
        session.peername = Z_strdup(ip);

    rootlen = MAX_OID_LEN;
    if (!snmp_parse_oid(oid, root, &rootlen)) {
        snmp_perror(oid);
		pthread_mutex_unlock(&snmp_lock);
        exit(1);
    }
    localdebug = netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                        NETSNMP_DS_LIB_DUMP_PACKET);

    get_field_names();
    reverse_fields();

    /*
     * open an SNMP session
     */
    SOCK_STARTUP;
    ss = snmp_open(&session);
    if (ss == NULL) {
        /*
         * diagnose snmp_open errors with the input netsnmp_session pointer
         */
        snmp_sess_perror("snmptable", &session);
		pthread_mutex_unlock(&snmp_lock);
        SOCK_CLEANUP;
		FREE_CLEANER
        exit(1);
    }

#ifndef NETSNMP_DISABLE_SNMPV1
    if (ss->version == SNMP_VERSION_1)
        use_getbulk = 0;
#endif

    do {
        entries = 0;
        allocated = 0;
        if (!headers_only) {
            if (use_getbulk)
                getbulk_table_entries(ss,entryptr, fieldptr, datastr);
            else
			{
                get_table_entries(ss);
			}
        }

        if (exitval) {
            snmp_close(ss);
            SOCK_CLEANUP;
			FREE_CLEANER
            return exitval;
        }

        if (indices) {
			for( int i = 0; i < entries ; i++){
				free(indices[i]);
			}
            free (indices);
            indices = NULL;
        }
        total_entries += entries;

    } while (!end_of_table);

    snmp_close(ss);
    SOCK_CLEANUP;
	FREE_CLEANER
	free(column);

    if (total_entries == 0)
        printf("%s: No entries\n", table_name);
    if (extra_columns)
	printf("%s: WARNING: More columns on agent than in MIB\n", table_name);

	free(table_name);
	table_name = NULL;
	pthread_mutex_unlock(&snmp_lock);

    return 0;
}

void
print_table(void)
{
	INIT_CLEANER
    int             entry, field, first_field, last_field = 0, width, part = 0;
    char          **dp;
    char            string_buf[SPRINT_MAX_LEN];
    char           *index_fmt = NULL;
    static int      first_pass = 1;

    if (!no_headers && !headers_only && first_pass)
        printf("SNMP table: %s\n\n", table_name);

    for (field = 0; field < fields; field++) {
        if (column_width != 0)
            sprintf(string_buf, "%%%s%d.%ds", left_justify_flag,
                    column_width + 1, column_width );
        else if (field_separator == NULL)
            sprintf(string_buf, "%%%s%ds", left_justify_flag,
                    column[field].width + 1);
        else if (field == 0 && !show_index)
            sprintf(string_buf, "%%s");
        else
            sprintf(string_buf, "%s%%s", field_separator);
        column[field].fmt = Z_strdup(string_buf);
    }
    if (show_index) {
        if (column_width)
            sprintf(string_buf, "\nindex: %%s\n");
        else if (field_separator == NULL)
            sprintf(string_buf, "%%%s%ds", left_justify_flag, index_width);
        else
            sprintf(string_buf, "%%s");
        index_fmt = Z_strdup(string_buf);
    }

    while (last_field != fields) {
        part++;
        if (part != 1 && !no_headers)
            printf("\nSNMP table %s, part %d\n\n", table_name, part);
        first_field = last_field;
        dp = data;
        if (show_index && !no_headers && !column_width && first_pass) {
            width = index_width;
            printf(index_fmt, "index");
        } else
            width = 0;
        for (field = first_field; field < fields; field++) {
            if (max_width)
            {
                if (column_width) {
                    if (!no_headers && first_pass) {
                        width += column_width + 1;
                        if (field != first_field && width > max_width) {
                            printf("\n");
                            width = column_width + 1;
                        }
                    }
                }
                else {
                    width += column[field].width + 1;
                    if (field != first_field && width > max_width)
                        break;
                }
            }
            if (!no_headers && first_pass)
                printf(column[field].fmt, column[field].label);
        }
        last_field = field;
        if (!no_headers && first_pass)
            printf("\n");
        for (entry = 0; entry < entries; entry++) {
            width = 0;
            if (show_index)
            {
                if (!column_width)
                    width = index_width;
                printf(index_fmt, indices[entry]);
            }
            for (field = first_field; field < last_field; field++) {
                if (column_width && max_width) {
                    width += column_width + 1;
                    if (field != first_field && width > max_width) {
                        printf("\n");
                        width = column_width + 1;
                    }
                }
                printf(column[field].fmt, dp[field] ? dp[field] : "?");
            }
            dp += fields;
            printf("\n");
        }
    }

    first_pass = 0;
	FREE_CLEANER
}

void
get_field_names(void)
{
	INIT_CLEANER
    char           *buf = NULL, *name_p = NULL;
    size_t          buf_len = 0, out_len = 0;
#ifndef NETSNMP_DISABLE_MIB_LOADING
    struct tree    *tbl = NULL;
#endif /* NETSNMP_DISABLE_MIB_LOADING */
    int             going = 1;

#ifndef NETSNMP_DISABLE_MIB_LOADING
    tbl = get_tree(root, rootlen, get_tree_head());
    if (tbl) {
        tbl = tbl->child_list;
        if (tbl) {
            root[rootlen++] = tbl->subid;
            tbl = tbl->child_list;
        } else {
            root[rootlen++] = 1;
            going = 0;
        }
    }
#endif /* NETSNMP_DISABLE_MIB_LOADING */

    if (sprint_realloc_objid
        ((u_char **)&buf, &buf_len, &out_len, 1, root, rootlen - 1)) {
        table_name = buf;
        buf = NULL;
        buf_len = out_len = 0;
    } else {
        table_name = Z_strdup("[TRUNCATED]");
        out_len = 0;
    }

    fields = 0;
    while (going) {
        fields++;
#ifndef NETSNMP_DISABLE_MIB_LOADING
        if (tbl) {
            if (tbl->access == MIB_ACCESS_NOACCESS) {
                fields--;
                tbl = tbl->next_peer;
                if (!tbl) {
                    going = 0;
                }
                continue;
            }
            root[rootlen] = tbl->subid;
            tbl = tbl->next_peer;
            if (!tbl)
                going = 0;
        } else {
#endif /* NETSNMP_DISABLE_MIB_LOADING */
            root[rootlen] = fields;
#ifndef NETSNMP_DISABLE_MIB_LOADING
        }
#endif /* NETSNMP_DISABLE_MIB_LOADING */
        out_len = 0;
        if (sprint_realloc_objid
            ((u_char **)&buf, &buf_len, &out_len, 1, root, rootlen + 1)) {
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
        if ('0' <= name_p[0] && name_p[0] <= '9') {
            fields--;
            break;
        }
        if (fields == 1) {
            column = (struct column *) malloc(sizeof(*column));
        } else {
            column =
                (struct column *) realloc(column,
                                          fields * sizeof(*column));
        }
        column[fields - 1].label = Z_strdup(name_p);
        column[fields - 1].width = strlen(name_p);
        column[fields - 1].subid = root[rootlen];
    }
    if (fields == 0) {
        fprintf(stderr, "Was that a table? %s\n", table_name);
        exit(1);
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
    }
    if (brief && fields > 1) {
        char           *f1, *f2;
        int             common = strlen(column[0].label);
        int             field, len;
        for (field = 1; field < fields; field++) {
            f1 = column[field - 1].label;
            f2 = column[field].label;
            while (*f1 && *f1++ == *f2++);
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
	FREE_CLEANER
}

void
get_table_entries(netsnmp_session * ss)
{
    int             running = 1;
    netsnmp_pdu    *pdu, *response;
    netsnmp_variable_list *vars;
    int             count;
    int             status;
    int             i;
    int             col;
    char           *buf = NULL;
    size_t          out_len = 0, buf_len = 0;
    char           *cp;
    char           *name_p = NULL;
    char          **dp;
    int             have_current_index;

    /*
     * TODO:
     *   1) Deal with multiple index fields
     *   2) Deal with variable length index fields
     *   3) optimize to remove a sparse column from get-requests
     */

    while (running &&
           ((max_width && !column_width) || (entries < max_getbulk))) {
        /*
         * create PDU for GETNEXT request and add object name to request
         */
        pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
        for (i = 1; i <= fields; i++) {
            name[rootlen] = column[i - 1].subid;
            snmp_add_null_var(pdu, name, name_length);
        }

        /*
         * do the request
         */
        status = snmp_synch_response(ss, pdu, &response);
        if (status == STAT_SUCCESS) {
            if (response->errstat == SNMP_ERR_NOERROR) {
                /*
                 * check resulting variables
                 */
                vars = response->variables;
                entries++;
                if (entries >= allocated) {
                    if (allocated == 0) {
                        allocated = 10;
                        data =
                            (char **) malloc(allocated * fields *
                                             sizeof(char *));
                        memset(data, 0,
                               allocated * fields * sizeof(char *));
                        if (show_index)
                            indices =
                                (char **) malloc(allocated *
                                                 sizeof(char *));
                    } else {
                        allocated += 10;
                        data =
                            (char **) realloc(data,
                                              allocated * fields *
                                              sizeof(char *));
                        memset(data + entries * fields, 0,
                               (allocated -
                                entries) * fields * sizeof(char *));
                        if (show_index)
                            indices =
                                (char **) realloc(indices,
                                                  allocated *
                                                  sizeof(char *));
                    }
                }
                dp = data + (entries - 1) * fields;
                col = -1;
                end_of_table = 1;       /* assume end of table */
                have_current_index = 0;
                name_length = rootlen + 1;
                for (vars = response->variables; vars;
                     vars = vars->next_variable) {
                    col++;
                    name[rootlen] = column[col].subid;
                    if ((vars->name_length < name_length) ||
                        (vars->name[rootlen] != column[col].subid) ||
                        memcmp(name, vars->name,
                               name_length * sizeof(oid)) != 0
                        || vars->type == SNMP_ENDOFMIBVIEW) {
                        /*
                         * not part of this subtree
                         */
                        if (localdebug) {
                            fprint_variable(stderr, vars->name,
                                            vars->name_length, vars);
                            fprintf(stderr, " => ignored\n");
                        }
                        continue;
                    }

                    /*
                     * save index off
                     */
                    if (!have_current_index) {
                        end_of_table = 0;
                        have_current_index = 1;
                        name_length = vars->name_length;
                        memcpy(name, vars->name,
                               name_length * sizeof(oid));
                        out_len = 0;
                        if (!sprint_realloc_objid
                            ((u_char **)&buf, &buf_len, &out_len, 1, vars->name,
                             vars->name_length)) {
                            break;
                        }
                        i = vars->name_length - rootlen + 1;
                        if (localdebug || show_index) {
                            if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                              NETSNMP_DS_LIB_EXTENDED_INDEX)) {
                                name_p = strchr(buf, '[');
                            } else {
                                switch (netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
                                                          NETSNMP_DS_LIB_OID_OUTPUT_FORMAT)) {
                                case NETSNMP_OID_OUTPUT_MODULE:
				case 0:
                                    name_p = strchr(buf, ':');
                                    break;
                                case NETSNMP_OID_OUTPUT_SUFFIX:
                                    name_p = buf;
                                    break;
                                case NETSNMP_OID_OUTPUT_FULL:
                                case NETSNMP_OID_OUTPUT_NUMERIC:
                                case NETSNMP_OID_OUTPUT_UCD:
                                    name_p = buf + strlen(table_name)+1;
                                    name_p = strchr(name_p, '.')+1;
                                    break;
				default:
				    fprintf(stderr, "Unrecognized -O option: %d\n",
					    netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
							      NETSNMP_DS_LIB_OID_OUTPUT_FORMAT));
				    exit(1);
                                }
                                name_p = strchr(name_p, '.') + 1;
                            }
                        }
                        if (localdebug) {
                            printf("Name: %s Index: %s\n", buf, name_p);
                        }
                        if (show_index) {
                            indices[entries - 1] = strdup(name_p);
                            i = strlen(name_p);
                            if (i > index_width)
                                index_width = i;
                        }
                    }

                    if (localdebug && buf) {
                        printf("%s => taken\n", buf);
                    }
                    out_len = 0;
                    sprint_realloc_value((u_char **)&buf, &buf_len, &out_len, 1,
                                         vars->name, vars->name_length,
                                         vars);
                    for (cp = buf; *cp; cp++) {
                        if (*cp == '\n') {
                            *cp = ' ';
                        }
                    }
                    dp[col] = buf;
                    i = out_len;
                    buf = NULL;
                    buf_len = 0;
                    if (i > column[col].width) {
                        column[col].width = i;
                    }
                }

                if (end_of_table) {
                    --entries;
                    /*
                     * not part of this subtree
                     */
                    if (localdebug) {
                        printf("End of table: %s\n",
                               buf ? (char *) buf : "[NIL]");
                    }
                    running = 0;
                    continue;
                }
            } else {
                /*
                 * error in response, print it
                 */
                running = 0;
                if (response->errstat == SNMP_ERR_NOSUCHNAME) {
                    printf("End of MIB\n");
                    end_of_table = 1;
                } else {
                    fprintf(stderr, "Error in packet.\nReason: %s\n",
                            snmp_errstring(response->errstat));
                    if (response->errindex != 0) {
                        fprintf(stderr, "Failed object: ");
                        for (count = 1, vars = response->variables;
                             vars && count != response->errindex;
                             vars = vars->next_variable, count++)
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
        } else {                /* status == STAT_ERROR */
            snmp_sess_perror("snmptable", ss);
            running = 0;
            exitval = 1;
        }
        if (response)
            snmp_free_pdu(response);
    }
}

void
getbulk_table_entries(netsnmp_session * ss,int* entryptr, int* fieldptr, char*** datastr)
{
    int             running = 1;
    netsnmp_pdu    *pdu, *response;
    netsnmp_variable_list *vars, *last_var;
    int             count;
    int             status;
    int             i;
    int             row, col;
    char           *buf = NULL;
    size_t          buf_len = 0, out_len = 0;
    char           *cp;
    char           *name_p = NULL;
    char          **dp;

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
        status = snmp_synch_response(ss, pdu, &response);
        if (status == STAT_SUCCESS) {
            if (response->errstat == SNMP_ERR_NOERROR) {
                /*
                 * check resulting variables
                 */
                vars = response->variables;
                last_var = NULL;
                while (vars) {
                    out_len = 0;
                    sprint_realloc_objid((u_char **)&buf, &buf_len, &out_len, 1,
                                         vars->name, vars->name_length);
                    if (vars->type == SNMP_ENDOFMIBVIEW ||
                        memcmp(vars->name, name,
                               rootlen * sizeof(oid)) != 0) {
                        if (localdebug) {
                            printf("%s => end of table\n",
                                   buf ? (char *) buf : "[NIL]");
                        }
                        running = 0;
                        break;
                    }
                    if (localdebug) {
                        printf("%s => taken\n",
                               buf ? (char *) buf : "[NIL]");
                    }
                    for (col = 0; col < fields; col++)
                        if (column[col].subid == vars->name[rootlen])
                            break;
					if (col == fields) {
						extra_columns = 1;
						last_var = vars;
						vars = vars->next_variable;
						continue;
		    		}
                    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                              NETSNMP_DS_LIB_EXTENDED_INDEX)) {
                        name_p = strchr(buf, '[');
                    } 
					else {
                        switch (netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
                                                  NETSNMP_DS_LIB_OID_OUTPUT_FORMAT)) {
                        case NETSNMP_OID_OUTPUT_MODULE:
						case 0:
                            name_p = strchr(buf, ':')+1;
                            break;
                        case NETSNMP_OID_OUTPUT_SUFFIX:
                            name_p = buf;
                            break;
                        case NETSNMP_OID_OUTPUT_FULL:
                        case NETSNMP_OID_OUTPUT_NUMERIC:
                        case NETSNMP_OID_OUTPUT_UCD:
                            name_p = buf + strlen(table_name)+1;
                            name_p = strchr(name_p, '.')+1;
                            break;
						default:
							fprintf(stderr, "Unrecognized -O option: %d\n",
							netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
					              NETSNMP_DS_LIB_OID_OUTPUT_FORMAT));
							exit(1);
                        }
						 name_p = strchr(name_p, '.');
						if ( name_p == NULL ) {
                            /* The 'strchr' call above failed, i.e. the results
                             * don't seem to include instance subidentifiers! */
                            running = 0;
                            break;
						}
						name_p++;  /* Move on to the instance identifier */
					}
                    for (row = 0; row < entries; row++)
                        if (strcmp(name_p, indices[row]) == 0)
                            break;
                    if (row == entries) {
                        entries++;
                        if (entries >= allocated) {
                            if (allocated == 0) {
                                allocated = 10;
                                data =
                                    (char **) malloc(allocated * fields *
                                                     sizeof(char *));
                                memset(data, 0,
                                       allocated * fields *
                                       sizeof(char *));
                                indices =
                                    (char **) malloc(allocated *
                                                     sizeof(char *));
                            } else {
                                allocated += 10;
                                data =
                                    (char **) realloc(data,
                                                      allocated * fields *
                                                      sizeof(char *));
                                memset(data + entries * fields, 0,
                                       (allocated -
                                        entries) * fields *
                                       sizeof(char *));
                                indices =
                                    (char **) realloc(indices,
                                                      allocated *
                                                      sizeof(char *));
                            }
                        }
                        indices[row] = strdup(name_p);
                        i = strlen(name_p);
                        if (i > index_width)
                            index_width = i;
                    }
                    dp = data + row * fields;
                    *datastr = data + row * fields;
                    out_len = 0;
                    sprint_realloc_value((u_char **)&buf, &buf_len, &out_len, 1,
                                         vars->name, vars->name_length,
                                         vars);
                    for (cp = buf; *cp; cp++)
                        if (*cp == '\n')
                            *cp = ' ';
                    dp[col] = buf;
					(*datastr)[col] = buf;
                    i = out_len;
                    buf = NULL;
                    buf_len = 0;
                    if (i > column[col].width)
                        column[col].width = i;
                    last_var = vars;
                    vars = vars->next_variable;
                }
                if (last_var) {
                    name_length = last_var->name_length;
                    memcpy(name, last_var->name,
                           name_length * sizeof(oid));
                }
            } else {
                /*
                 * error in response, print it
                 */
                running = 0;
                if (response->errstat == SNMP_ERR_NOSUCHNAME) {
                    printf("End of MIB\n");
                } else {
                    fprintf(stderr, "Error in packet.\nReason: %s\n",
                            snmp_errstring(response->errstat));
                    if (response->errstat == SNMP_ERR_NOSUCHNAME) {
                        fprintf(stderr,
                                "The request for this object identifier failed: ");
                        for (count = 1, vars = response->variables;
                             vars && count != response->errindex;
                             vars = vars->next_variable, count++)
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
        } else {                /* status == STAT_ERROR */
            snmp_sess_perror("snmptable", ss);
            running = 0;
            exitval = 1;
        }
        if (response)
            snmp_free_pdu(response);
    }
	*fieldptr = fields;
	*entryptr = entries;
	*datastr = data;
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
