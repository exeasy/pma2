/*
 * snmpwalk.c
 *
 *  Created on: Dec 24, 2012
 *      Author: liupengzhan
 */


#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>

int snmpget1(char * address,char * community, char * oidNO ,char * rs)
{
struct snmp_session session;
struct snmp_session *sess_handle;

struct snmp_pdu *pdu;
struct snmp_pdu *response;

struct variable_list *vars;

oid id_oid[MAX_OID_LEN];
oid serial_oid[MAX_OID_LEN];

oid anOID[MAX_OID_LEN];

size_t id_len = MAX_OID_LEN;
//size_t serial_len = MAX_OID_LEN;

size_t anOID_len = MAX_OID_LEN;

int status;
int count=1;
char soid[100] = "\0";
//int snmpsetvalue=-1;

//struct tree * mib_tree;

/*连接的主机名*/
//char * address = "192.168.1.240";

init_snmp("snmpd");

/*首先，需要初始化一个snmp会话，snmp_sess_init函数完成这一工作，它的参数是一个struct snmp_session结构变量，它初始化该结构的大多数成员的默认值，初始化完成后，我们自定义一些必要的成员字段*/
snmp_sess_init( &session );
session.version = SNMP_VERSION_2c;
session.community = strdup(community);
session.community_len = (size_t)strlen(session.community);
session.peername = strdup(address);
/*snmp_open打开snmp会话，它的参数是我们刚才初始化的结构，返回一个struct snmp_session类型指针*/
SOCK_STARTUP;
sess_handle = snmp_open(&session);                     /* establish the session */
if (!sess_handle) {
  snmp_sess_perror("ack", &session);
  SOCK_CLEANUP;
  exit(1);
}

/*创建PDU（PDU即协议数据单元，也就是分组）*/
pdu = snmp_pdu_create(SNMP_MSG_GET);
strcat(soid, oidNO);
strcat(soid, ".0");
if (!snmp_parse_oid(soid, anOID, &anOID_len)) {
  snmp_perror(oidNO);
  SOCK_CLEANUP;
  exit(1);
}
snmp_add_null_var(pdu, anOID, anOID_len);

/*我们需要读取我们想要获取的对像的ID，read_objid函数有三个参数，第一个参数是要读取的对像的ID的字符串表示形式，“MIB库：：名称.索引”的形式，第二个参数代表了对像ID，第三个参数是长度*/
read_objid("SNMPv2-MIB::sysDescr.0", id_oid, &id_len);
///*把读取到的对像ID，添加进请求数据分组包中，这样，一个请求包就构建完成了，可以反复地构建多个请求对像*/
snmp_add_null_var(pdu,id_oid,id_len);
//
read_objid("IP-MIB::ipInReceives.0",id_oid,&id_len);
snmp_add_null_var(pdu,id_oid,id_len);

//snmp_parse_oid(".1.3.6.1.2.1.4.20.1.2.127.0.0.1", anOID , &anOID_len);
//snmp_add_null_var(pdu,anOID,anOID_len);

/*发送SNMP请求*/
status = snmp_synch_response(sess_handle, pdu, &response);

/*处理agent返回的信息*/
 /*
  * Process the response.
  */
 if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
   /*
    * SUCCESS: Print the result variables
    */

   for(vars = response->variables; vars; vars = vars->next_variable)
     print_variable(vars->name, vars->name_length, vars);

   vars = response->variables;
   snprint_variable(rs, 1024, vars->name, vars->name_length, vars);

   vars = response->variables;
   if(vars->type == ASN_OCTET_STR ){
	   memcpy(rs, vars->val.string, vars->val_len);
   }
   if (vars->type == ASN_IPADDRESS) {
	   u_char         *ip = vars->val.string;
		if (ip)
			sprintf(rs, "%d.%d.%d.%d", ip[0],ip[1], ip[2], ip[3]);
		printf("=%s",rs);
   }
   /* manipulate the information ourselves */
//   for(vars = response->variables; vars; vars = vars->next_variable) {
//     if (vars->type == ASN_OCTET_STR) {
//	  char *sp = (char *)malloc(1 + vars->val_len);
//	  memcpy(sp, vars->val.string, vars->val_len);
//
//	  sp[vars->val_len] = '\0';
//      printf("value #%d is a string: %s\n", count++, sp);
//	  free(sp);
//	}
//     else
//       printf("value #%d is NOT a string! Ack!\n", count++);
//   }
 } else {
   /*
    * FAILURE: print what went wrong!
    */

   if (status == STAT_SUCCESS)
     fprintf(stderr, "Error in packet\nReason: %s\n",
             snmp_errstring(response->errstat));
   else if (status == STAT_TIMEOUT)
     fprintf(stderr, "Timeout: No response from %s.\n",
             session.peername);
   else
     snmp_sess_perror("snmpdemoapp", sess_handle);

 }

 /*
  * Clean up:
  *  1) free the response.
  *  2) close the session.
  */
 if (response)
   snmp_free_pdu(response);
 snmp_close(sess_handle);

 SOCK_CLEANUP;
 return (0);
}


