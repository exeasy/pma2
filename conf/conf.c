#include <utils/common.h>
#include <utils/utils.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <conf.h>

/* 默认的hello定时器
*  high_hello 1
*  low_hello  20
*  high_dead 4
*  low_daed 80
*/
#define DEFAULT_H_HELLO 1
#define DEFAULT_L_HELLO 20
#define DEFAULT_H_DEAD 4
#define DEFAULT_L_DEAD 80

/* pma 配置文件 */
#define PMA_CONF "pma.conf"

/* 配置文件字段 */
#define VERSION 		"version"
#define PMA_ID		"pma_id"
#define AS_NUM		"as_num"
#define POLICY_TYPE		"policy_type"
#define SNAPSHOT_TIME	"snapshot_timeval"
#define LISTEN_PORT		"listen_port"
#define SERVER			"server"
#define ALCA 		"alca"
#define LOGSRV 		"logsrv"
#define RLTM        "rltm"
#define IP				"ip"
#define PORT			"port"
#define MODULE			"module"
#define IC_S				"ic"
#define DBM_S				"dbm"
#define PEA_S				"pem"
#define OUTSIDE_ENABLE				"outside_enable"
#define DEVICE_TYPE		"device_type"
#define ROUTER_IP		"router_ip"
#define LOCAL_IP        "local_ip"
#define NETMASK         "netmask"
#define COMM_TYPE          "comm_type"
#define FAST_MPLS	"fast_mpls"

#define GET_MOD_CONFIG(x) pma_conf.x##_config


struct conf pma_conf =
{
	.version = 0,
	.pma_id = 0,
	.server_port = 0,
	.listen_port = 0,
	.comm_type = 0,
	.server_ip = {0},
};

char *conf_file = NULL;

/* Get the Item Under the Node(node) which name is 'name'
 * then store it to the 'dst' . sz present the size of dst
 * type 0: present the number
 *      1: present the string
 *      2: present the IP address
 */
static int parase_item(xmlDocPtr doc, xmlNodePtr node, const xmlChar *name, void* dst, size_t sz, int type)
{
	xmlNodePtr cur_node = NULL;
	xmlChar *cur_data = NULL;
	char *endptr = NULL;
	int base = 10;

	cur_data = get_value_by_name(doc, node, name );
	if ( cur_data == NULL)
	{
		DEBUG(ERROR,"%s ERROR", name);
		return -1;
	}
	DEBUG(INFO, "%s: %s",name, cur_data);
	switch ( type ){
		case 0:// Number
			if (sz == 4 )
				*(int*)dst = strtol( (char*)cur_data, &endptr, base);
			else if ( sz == 2 )
				*(short*)dst = strtol( (char*)cur_data, &endptr, base);
			else if ( sz == 1 )
				*(unsigned char*)dst = strtol( (char*)cur_data, &endptr, base);
			if ((errno == ERANGE )
				  || (errno != 0 && *(int*)dst < 0)) {
				DEBUG(ERROR, "get value %s",name);
				return -1;
			}
			xmlFree(cur_data);
			break;
		case 1:// String
			memset(dst, 0, sz);
			memcpy(dst, cur_data, strlen((char *)cur_data));
			xmlFree(cur_data);
			break;
		case 2:// IP address
			inet_pton(AF_INET,cur_data,(int*)dst);
			*(int*)dst = ntohl(*(int*)dst);
			xmlFree(cur_data);
			break;
		case 3:// IP ADDRESS
			inet_pton(AF_INET,cur_data,(int*)dst);
			xmlFree(cur_data);
			break;
		default:break;
	}
	return 0;
}


static int parse_conf(xmlDocPtr doc, xmlNodePtr node)
{
	xmlNodePtr cur_node = NULL;
	int ret = 0;
	/** device_type(11,12,13,21,22,23) **/   
	ret = parase_item(doc, node, (const xmlChar*)(DEVICE_TYPE), &pma_conf.device_type, sizeof(int), 0);
	if(ret) return -1;
	/* comm_type */
	ret = parase_item(doc, node, (const xmlChar*)(COMM_TYPE), &pma_conf.comm_type, sizeof(int), 0);
	if(ret) return -1;
	/* version */
	ret = parase_item(doc, node, (const xmlChar*)(VERSION), &pma_conf.version, sizeof(int), 0);
	if(ret) return -1;
	/* pma_id */
	ret = parase_item(doc, node, (const xmlChar*)(PMA_ID), &pma_conf.pma_id, sizeof(int), 2);
	if(ret) return -1;
	/* as_num */
	ret = parase_item(doc, node, (const xmlChar*)(AS_NUM), &pma_conf.as_num, sizeof(int), 0);
	if(ret) return -1;
	/* listen_port */
	ret = parase_item(doc, node, (const xmlChar*)(LISTEN_PORT), &pma_conf.listen_port, sizeof(int), 0);
	if(ret) return -1;
	/* PMS Server INFO */
	cur_node = get_node_by_name(doc ,node, (const xmlChar*)(SERVER));
	/** PMS IP **/
	ret = parase_item(doc, cur_node, (const xmlChar*)(IP), pma_conf.server_ip, 46, 1);
	if(ret) return -1;
	/** PMS PORT **/
	ret = parase_item(doc, cur_node, (const xmlChar*)(PORT), &pma_conf.server_port, sizeof(int), 0);
	if(ret) return -1;
	/* ALCA Server INFO */
	cur_node = get_node_by_name(doc ,node, (const xmlChar*)(ALCA));
	/** ALCA IP **/
	ret = parase_item(doc, cur_node, (const xmlChar*)(IP), pma_conf.alca_ip, 46, 1);
	if(ret) return -1;
	/** ALCA PORT **/
	ret = parase_item(doc, cur_node, (const xmlChar*)(PORT), &pma_conf.alca_port, sizeof(int), 0);
	if(ret) return -1;
	/* Log Server INFO */
	cur_node = get_node_by_name(doc ,node, (const xmlChar*)(LOGSRV));
	/** LogSrv IP **/
	ret = parase_item(doc, cur_node, (const xmlChar*)(IP), pma_conf.logsrv_ip, 46, 1);
	if(ret) return -1;
	/** LogSrv PORT **/
	ret = parase_item(doc, cur_node, (const xmlChar*)(PORT), &pma_conf.logsrv_port, sizeof(int), 0);
	if(ret) return -1;
	/* RLTM Server INFO */
	cur_node = get_node_by_name(doc ,node, (const xmlChar*)(RLTM));
	/** RLTM IP **/
	ret = parase_item(doc, cur_node, (const xmlChar*)(IP), pma_conf.rltm_ip, 46, 1);
	if(ret) return -1;
	/** RLTM PORT **/
	ret = parase_item(doc, cur_node, (const xmlChar*)(PORT), &pma_conf.rltm_port, sizeof(int), 0);
	if(ret) return -1;
	/* PMA Module Configuration */
	cur_node = get_node_by_name(doc, node, (const xmlChar*)(MODULE));
	/* ICModule */
	xmlNodePtr m_node = get_node_by_name(doc, cur_node, (const xmlChar*)(IC_S));
	/** outside_enable **/
	ret = parase_item(doc, m_node, (const xmlChar*)(OUTSIDE_ENABLE), &(GET_MOD_CONFIG(ic).outside), sizeof(int), 0);
	if(ret) return -1;
	/** router ip **/
	ret = parase_item(doc, m_node, (const xmlChar*)(ROUTER_IP), &(GET_MOD_CONFIG(ic).router_ip), sizeof(int), 3);
	if(ret) return -1;
	/** local ip **/
	ret = parase_item(doc, m_node, (const xmlChar*)(LOCAL_IP), &(GET_MOD_CONFIG(ic).local_ip), sizeof(int), 3);
	if(ret) return -1;
	/** netmask **/
	ret = parase_item(doc, m_node, (const xmlChar*)(NETMASK), &(GET_MOD_CONFIG(ic).netmask), sizeof(int), 3);
	if(ret) return -1;
	/* DBModule */
	m_node = get_node_by_name(doc, cur_node, (const xmlChar*)(DBM_S));
	/* snapshoot timeval */
	ret = parase_item(doc, m_node, (const xmlChar*)(SNAPSHOT_TIME), &(GET_MOD_CONFIG(dbm).snapshoot_timeval), sizeof(int), 0);
	if(ret) return -1;
	/* policy type */
	ret = parase_item(doc, m_node, (const xmlChar*)(POLICY_TYPE), &(GET_MOD_CONFIG(dbm).policy_type), sizeof(int), 0);
	if(ret) return -1;
	/* PEModule */
	m_node = get_node_by_name(doc, cur_node, (const xmlChar*)(PEA_S));
	/* fast_mpls */
	ret = parase_item(doc, m_node, (const xmlChar*)(FAST_MPLS), &(GET_MOD_CONFIG(pea).fast_mpls), sizeof(int), 0);
	if(ret) return -1;

	GET_MOD_CONFIG(ic).device_type = pma_conf.device_type;
	GET_MOD_CONFIG(pea).device_type = pma_conf.device_type;
	GET_MOD_CONFIG(ic).pma_id = pma_conf.pma_id;
	GET_MOD_CONFIG(dbm).pma_id = pma_conf.pma_id;
	GET_MOD_CONFIG(pea).pma_id = pma_conf.pma_id;
	GET_MOD_CONFIG(pea).router_ip = GET_MOD_CONFIG(ic).router_ip;
	return 0;
}

static int parse_conf_old(xmlDocPtr doc, xmlNodePtr node)
{
	xmlNodePtr cur_node = NULL;
	xmlNodePtr child_node = NULL;
	xmlNodePtr cc_node = NULL;


	char *endptr = NULL;
	int base = 10;

	for (cur_node = node->children; cur_node!= NULL; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			if (xmlStrcmp(cur_node->name, (const xmlChar *)(VERSION)) == 0) {
				xmlChar * value = xmlNodeListGetString(doc, cur_node->xmlChildrenNode, 1);
				DEBUG(INFO, "%s: %s",cur_node->name, value);
				pma_conf.version = strtol((char *)value, &endptr, base);
				if ((errno == ERANGE )
						|| (errno != 0 && pma_conf.version == 0)) {
					DEBUG(ERROR, "get value version");
					return -1;
				}
				xmlFree(value);
			}else if (xmlStrcmp(cur_node->name, (const xmlChar *)(PMA_ID)) == 0) {
				xmlChar * value = xmlNodeListGetString(doc, cur_node->xmlChildrenNode, 1);
#ifndef OSPF_VERSION
				inet_pton(AF_INET,value,&pma_conf.pma_id);
				pma_conf.pma_id = ntohl(pma_conf.pma_id);
#endif
#ifdef OSPF_VERSION
				DEBUG(INFO, "%s: %d",cur_node->name, pma_conf.pma_id);
				pma_conf.pma_id = strtol((char *)value, &endptr, base);
#endif
				if ((errno == ERANGE )
						|| (errno != 0 && pma_conf.pma_id == 0)) {
					DEBUG(ERROR, "get value agent_id");
					return -1;
				}
				xmlFree(value);
			}
			else if (xmlStrcmp(cur_node->name, (const xmlChar *)(LISTEN_PORT)) == 0) {
				  xmlChar * value = xmlNodeListGetString(doc, cur_node->xmlChildrenNode, 1);
				  DEBUG(INFO, "%s: %s",cur_node->name, value);
				  pma_conf.listen_port = strtol((char *)value, &endptr, base);
				  if ((errno == ERANGE )
						  || (errno != 0 && pma_conf.listen_port == 0)) {
					  DEBUG(ERROR, "get value listen_port");
					  return -1;
				  }
				  xmlFree(value);
			  }else if (xmlStrcmp(cur_node->name, (const xmlChar *)(COMM_TYPE)) == 0) {
				  xmlChar * value = xmlNodeListGetString(doc, cur_node->xmlChildrenNode, 1);
				  DEBUG(INFO, "%s: %s",cur_node->name, value);
				  pma_conf.comm_type = strtol((char *)value, &endptr, base);
				  if ((errno == ERANGE )
						  || (errno != 0 && pma_conf.comm_type < 0)) {
					  DEBUG(ERROR, "get value Comm_type");
					  return -1;
				  }
				  xmlFree(value);
			  }else if (xmlStrcmp(cur_node->name, (const xmlChar *)(SERVER)) == 0) {
				  for (child_node = cur_node->children; child_node!= NULL; child_node = child_node->next) {
					  if (child_node->type == XML_ELEMENT_NODE) {
						  if (xmlStrcmp(child_node->name, (const xmlChar *)(IP)) == 0) {
							  xmlChar * value = xmlNodeListGetString(doc, child_node->xmlChildrenNode, 1);
							  DEBUG(INFO, "%s: %s",child_node->name, value);
							  memset(pma_conf.server_ip, 0, sizeof(pma_conf.server_ip));
							  memcpy(pma_conf.server_ip, value, strlen((char *)value));
							  xmlFree(value);
						  }
						  if (xmlStrcmp(child_node->name, (const xmlChar *)(PORT)) == 0) {
							  xmlChar * value = xmlNodeListGetString(doc, child_node->xmlChildrenNode, 1);
							  DEBUG(INFO, "%s: %s",child_node->name, value);
							  pma_conf.server_port = strtol((char *)value, &endptr, base);
							  if ((errno == ERANGE )
									  || (errno != 0 && pma_conf.server_port == 0)) {
								  DEBUG(ERROR, "get value server_port");
								  return -1;
							  }
							  xmlFree(value);
						  }
					  }
				  }
			  }
			  else if (xmlStrcmp(cur_node->name, (const xmlChar *)(ALCA)) == 0) {
				  for (child_node = cur_node->children; child_node!= NULL; child_node = child_node->next) {
					  if (child_node->type == XML_ELEMENT_NODE) {
						  if (xmlStrcmp(child_node->name, (const xmlChar *)(IP)) == 0) {
							  xmlChar * value = xmlNodeListGetString(doc, child_node->xmlChildrenNode, 1);
							  DEBUG(INFO, "%s: %s",child_node->name, value);
							  memset(pma_conf.alca_ip, 0, sizeof(pma_conf.alca_ip));
							  memcpy(pma_conf.alca_ip, value, strlen((char *)value));
							  xmlFree(value);
						  }
						  if (xmlStrcmp(child_node->name, (const xmlChar *)(PORT)) == 0) {
							  xmlChar * value = xmlNodeListGetString(doc, child_node->xmlChildrenNode, 1);
							  DEBUG(INFO, "%s: %s",child_node->name, value);
							  pma_conf.alca_port = strtol((char *)value, &endptr, base);
							  if ((errno == ERANGE )
									  || (errno != 0 && pma_conf.alca_port == 0)) {
								  DBUG(ERROR, "get value ALCA_port");
								  return -1;
							  }
							  xmlFree(value);
						  }
					  }
				  }
			  }
			  else if (xmlStrcmp(cur_node->name, (const xmlChar *)(RLTM)) == 0) {
				  for (child_node = cur_node->children; child_node!= NULL; child_node = child_node->next) {
					  if (child_node->type == XML_ELEMENT_NODE) {
						  if (xmlStrcmp(child_node->name, (const xmlChar *)(IP)) == 0) {
							  xmlChar * value = xmlNodeListGetString(doc, child_node->xmlChildrenNode, 1);
							  DEBUG(INFO, "%s: %s",child_node->name, value);
							  memset(pma_conf.rltm_ip, 0, sizeof(pma_conf.rltm_ip));
							  memcpy(pma_conf.rltm_ip, value, strlen((char *)value));
							  xmlFree(value);
						  }
						  if (xmlStrcmp(child_node->name, (const xmlChar *)(PORT)) == 0) {
							  xmlChar * value = xmlNodeListGetString(doc, child_node->xmlChildrenNode, 1);
							  DEBUG(INFO, "%s: %s",child_node->name, value);
							  pma_conf.rltm_port = strtol((char *)value, &endptr, base);
							  if ((errno == ERANGE )
									  || (errno != 0 && pma_conf.rltm_port == 0)) {
								  DEBUG(ERROR, "get value RLTM_port");
								  return -1;
							  }
							  xmlFree(value);
						  }
					  }
				  }
			  }
			  else if (xmlStrcmp(cur_node->name, (const xmlChar *)(MODULE)) == 0) {
				  struct in_addr router_ip;
				  struct in_addr local_ip;
				  struct in_addr netmask;
				  for (child_node = cur_node->children; child_node!= NULL; child_node = child_node->next) {
					  if (child_node->type == XML_ELEMENT_NODE) {
						  if (xmlStrcmp(child_node->name, (const xmlChar *)(IC_S)) == 0) {

							  for (cc_node = child_node->children; cc_node!= NULL; cc_node = cc_node->next)
							  {
								  if (xmlStrcmp(cc_node->name, (const xmlChar *)(ROUTER_IP)) == 0) {
									  xmlChar * value = xmlNodeListGetString(doc, cc_node->xmlChildrenNode, 1);
									  DEBUG(INFO,  "%s: %s",cc_node->name, value);

									  inet_pton(AF_INET,value,&(GET_MOD_CONFIG(ic).router_ip));//add by Macro
									  //router_id = strtol((char *)value, &endptr, base);//modify by macro
									  if ((errno == ERANGE )
											  || (errno != 0 && (GET_MOD_CONFIG(ic).router_ip).s_addr == 0)) {
										  perror("get value router_ip");
										  return -1;
									  }
									  xmlFree(value);
								  }
								  else if (xmlStrcmp(cc_node->name, (const xmlChar *)(LOCAL_IP)) == 0) {
									  xmlChar * value = xmlNodeListGetString(doc, cc_node->xmlChildrenNode, 1);
									  DEBUG(INFO, "%s: %s",cc_node->name, value);
									  //memset(ip, 0, sizeof(ip));
									  //memcpy(ip, value, strlen((char *)value));
									  inet_pton(AF_INET,value,&(GET_MOD_CONFIG(ic).local_ip));
									  xmlFree(value);
								  }
								  else if (xmlStrcmp(cc_node->name, (const xmlChar *)(NETMASK)) == 0) {
									  xmlChar * value = xmlNodeListGetString(doc, cc_node->xmlChildrenNode, 1);
									  DEBUG(INFO, "%s: %s",cc_node->name, value);
									  inet_pton(AF_INET,value,&(GET_MOD_CONFIG(ic).netmask));
									  //port = strtol((char *)value, &endptr, base);
									  if ((errno == ERANGE )
											  || (errno != 0 && (GET_MOD_CONFIG(ic).netmask).s_addr == 0)) {
										  DEBUG(ERROR, "get value local ip netmask");
										  return -1;
									  }
									  xmlFree(value);
								  }
							  }
							  GET_MOD_CONFIG(ic).pma_id = pma_conf.pma_id;
						  }
						  else if(xmlStrcmp(child_node->name, (const xmlChar *)(DBM_S)) == 0){

							  for (cc_node = child_node->children; cc_node!= NULL; cc_node = cc_node->next)
							  {
								  if (xmlStrcmp(cc_node->name, (const xmlChar *)(SNAPSHOT_TIME)) == 0) {
									  xmlChar * value = xmlNodeListGetString(doc, cc_node->xmlChildrenNode, 1);
									  DEBUG(INFO, "%s: %s",cc_node->name, value);
									  GET_MOD_CONFIG(dbm).snapshoot_timeval = strtol((char *)value, &endptr, base);
									  if ((errno == ERANGE )
											  || (errno != 0 && GET_MOD_CONFIG(dbm).snapshoot_timeval == 0)) {
										  DEBUG(ERROR, "get value snapshot_timeval");
										  return -1;
									  }
									  xmlFree(value);
								  }
								  else if (xmlStrcmp(cc_node->name, (const xmlChar *)(POLICY_TYPE)) == 0) {
									  xmlChar * value = xmlNodeListGetString(doc, cc_node->xmlChildrenNode, 1);
									  DEBUG(INFO, "%s: %s",cc_node->name, value);
									  GET_MOD_CONFIG(dbm).policy_type = strtol((char *)value, &endptr, base);
									  if ((errno == ERANGE )
											  || (errno != 0 && GET_MOD_CONFIG(dbm).policy_type == 0)) {
										  DEBUG(ERROR, "get value policy_type");
										  return -1;
									  }
									  xmlFree(value);
								  }
							  }
							  GET_MOD_CONFIG(dbm).pma_id = pma_conf.pma_id;
							  GET_MOD_CONFIG(pea).pma_id = pma_conf.pma_id;
							  GET_MOD_CONFIG(pea).router_ip = GET_MOD_CONFIG(ic).router_ip;
						  }
					  }
				  }
			  }
		}
	}
	return 0;
}

int conf_init()
{
	char *conf = NULL;

	if (conf_file == NULL) {
		conf = PMA_CONF;
	} else {
		conf = conf_file;
	}

	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;

	doc = xmlParseFile(conf);

	if (doc == NULL) {
		DEBUG(ERROR, "could not open the config file(%s)", conf);
		return -1;
	}

	root_node = xmlDocGetRootElement(doc);

	if (parse_conf(doc, root_node) == -1) {
		DEBUG(ERROR, "parse the config file(%s) failed", conf);
		return -1;
	}

	pma_conf.ic_config.if_id = -1;
	pma_conf.ic_config.h_hello_val = DEFAULT_H_HELLO;
	pma_conf.ic_config.l_hello_val = DEFAULT_L_HELLO;
	pma_conf.ic_config.h_dead_val = DEFAULT_H_DEAD;
	pma_conf.ic_config.l_dead_val = DEFAULT_L_DEAD;


	xmlFreeDoc(doc);
	xmlCleanupParser();

	return 0;
}

int get_protocol_type()
{
	return pma_conf.device_type % 10;
}

int get_device_type()
{
	return pma_conf.device_type/10;
}

int get_version(void)
{
	return pma_conf.version;
}
int get_pma_id(void)
{
	return pma_conf.pma_id;
}

int get_pmaca(void)
{
	return 0;
}

char* get_alca_address(void)
{
	return pma_conf.alca_ip;
}

int get_alca_port(void)
{
	return pma_conf.alca_port;
}
char* get_logsrv_address(void)
{
	return pma_conf.logsrv_ip;
}

int get_logsrv_port(void)
{
	return pma_conf.logsrv_port;
}
char* get_rltm_address(void)
{
	return pma_conf.rltm_ip;
}

int get_rltm_port(void)
{
	return pma_conf.rltm_port;
}
int get_local_ip(void)
{
	return pma_conf.ic_config.local_ip.s_addr;
}

char *get_server_address(void)
{
	return pma_conf.server_ip;
}

int get_timeval(void){return 5;}

int get_server_port(void)
{
	return pma_conf.server_port;
}
int get_listen_port(void)
{
	return pma_conf.listen_port;
}

int get_comm_type(void)
{
	return pma_conf.comm_type;
}
