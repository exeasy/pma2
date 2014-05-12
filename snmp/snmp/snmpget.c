#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>
#include <includes/snmpget.h>


int snmpget(char * address, char * community, char * oidNO, char * rs) {
	INIT_CLEANER
	struct snmp_session session;
	struct snmp_session *sess_handle;

	struct snmp_pdu *pdu;
	struct snmp_pdu *response;

	struct variable_list *vars;

	oid id_oid[MAX_OID_LEN];
	oid serial_oid[MAX_OID_LEN];

	oid anOID[MAX_OID_LEN];

	size_t id_len = MAX_OID_LEN;

	size_t anOID_len = MAX_OID_LEN;

	int status;
	int count = 1;

	init_snmp("snmpd");

	/*首先，需要初始化一个snmp会话，snmp_sess_init函数完成这一工作，它的参数是一个struct snmp_session结构变量，它初始化该结构的大多数成员的默认值，初始化完成后，我们自定义一些必要的成员字段*/
	snmp_sess_init(&session);
	session.version = SNMP_VERSION_2c;
	session.community = Z_strdup(community);
	session.community_len = (size_t) strlen(session.community);
	session.peername = Z_strdup(address);

	/*snmp_open打开snmp会话，它的参数是我们刚才初始化的结构，返回一个struct snmp_session类型指针*/
	SOCK_STARTUP;
	sess_handle = snmp_sess_open(&session); /* establish the session */
	if (!sess_handle) {
		snmp_sess_perror("ack", &session);
		SOCK_CLEANUP;
		FREE_CLEANER
		return -1;
	}

	/*创建PDU（PDU即协议数据单元，也就是分组）*/
	pdu = snmp_pdu_create(SNMP_MSG_GET);
	if (!snmp_parse_oid(oidNO, anOID, &anOID_len)) {
		snmp_perror(oidNO);
		snmp_sess_close(sess_handle);
		SOCK_CLEANUP;
		FREE_CLEANER
		return -1;
	}
	snmp_add_null_var(pdu, anOID, anOID_len);

	/*我们需要读取我们想要获取的对像的ID，read_objid函数有三个参数，第一个参数是要读取的对像的ID的字符串表示形式，“MIB库：：名称.索引”的形式，第二个参数代表了对像ID，第三个参数是长度*/
	//read_objid("SNMPv2-MIB::sysDescr.0", id_oid, &id_len);
	///*把读取到的对像ID，添加进请求数据分组包中，这样，一个请求包就构建完成了，可以反复地构建多个请求对像*/
	//snmp_add_null_var(pdu, id_oid, id_len);
	//
	//read_objid("IP-MIB::ipInReceives.0", id_oid, &id_len);
	//snmp_add_null_var(pdu, id_oid, id_len);

	//snmp_parse_oid(".1.3.6.1.2.1.4.20.1.2.127.0.0.1", anOID , &anOID_len);
	//snmp_add_null_var(pdu,anOID,anOID_len);

	/*发送SNMP请求*/
	if (sess_handle == NULL) {
		snmp_sess_perror("ack", &session);
		SOCK_CLEANUP;
		FREE_CLEANER
		return -1;
	}
	status = snmp_sess_synch_response(sess_handle, pdu, &response);

	/*处理agent返回的信息*/
	/*
	 * Process the response.
	 */
	if (status == STAT_SUCCESS ) {
		/*
		 * SUCCESS: Print the result variables
		 */
		if (response == NULL) {
			snmp_sess_close(sess_handle);
			SOCK_CLEANUP;
			return -1;
		}
		if (response->errstat == SNMP_ERR_NOERROR) {
			vars = response->variables;
			switch (vars->type) {
				case ASN_OCTET_STR:
					memcpy(rs, vars->val.string, vars->val_len);
					break;
				case ASN_IPADDRESS: {
					u_char *ip = vars->val.string;
					if (ip) {
						sprintf(rs, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
					}
					break;
				}
				case ASN_INTEGER: {
					sprintf(rs, "%d", *(vars->val.integer));
					break;
				}
				case ASN_COUNTER: {
					sprintf(rs, "%u", (unsigned int) (*vars->val.integer
							& 0xffffffff));
					break;
				}
			}
		}
	} 
	else {
		/*
		 * FAILURE: print what went wrong!
		 */

		if (status == STAT_SUCCESS)
			fprintf(stderr, "Error in packet\nReason: %s\n", snmp_errstring(
					response->errstat));
		else if (status == STAT_TIMEOUT)
			fprintf(stderr, "Timeout: No response from %s.\n", session.peername);
		else
			snmp_sess_perror("snmpdemoapp", sess_handle);
		if (response)
			snmp_free_pdu(response);
		snmp_sess_close(sess_handle);
		SOCK_CLEANUP;
		return -1;

	}

	/*
	 * Clean up:
	 *  1) free the response.
	 *  2) close the session.
	 */
	if (response)
		snmp_free_pdu(response);
	snmp_sess_close(sess_handle);
	SOCK_CLEANUP;
	FREE_CLEANER
	return (0);
}

