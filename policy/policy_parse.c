#include <utils/common.h>
#include <utils/utils.h>
#include <utils/xml.h>
#include <policy.h>
#include <policy_table.h>
#include <policy_parse.h>

extern struct _policy_table *g_pt;

static int parse_policy( xmlDocPtr doc, xmlNodePtr node)
{
	int version = 0;
	int operation = 0;
	int up_limit = 0;
	int lower_limit = 0;
	unsigned long long value1 = 0;
	unsigned long long value2 = 0;

	char *endptr = NULL;
	int base = 10;

#define INIT_VARS() \
	({\
	 operation = 0;\
	 up_limit = 0;\
	 lower_limit = 0;\
	 value1 = 0;\
	 value2 = 0;\
	 })

	xmlNodePtr cur_node = NULL;
	xmlChar *cur_data = NULL;

	cur_data = get_value_by_name( doc, node, (xmlChar*)(VERSION));
	if(cur_data == NULL)
	{
		DEBUG(ERROR,"Get version failed");
		return -1;
	}
	version = strtol((char *)cur_data, &endptr, base);
	xmlFree(cur_data);

	for ( cur_node = node->children; cur_node != NULL; cur_node = cur_node->next)
	{
		if( xmlStrcmp(cur_node->name, (const xmlChar*)(VERSION)) == 0)
		{
			continue;
		}
		else
		{
			cur_data = get_value_by_name(doc, cur_node, (const xmlChar*)(OPERATION));
			operation = strtol((char *)cur_data, &endptr, base);
			xmlFree(cur_data);
			cur_data = get_value_by_name(doc, cur_node, (const xmlChar*)(UP_LIMIT));
			up_limit = strtol((char *)cur_data, &endptr, base);
			xmlFree(cur_data);
			cur_data = get_value_by_name(doc, cur_node, (const xmlChar*)(LOWER_LIMIT));
			lower_limit = strtol((char *)cur_data, &endptr, base);
			xmlFree(cur_data);
			cur_data = get_value_by_name(doc, cur_node, (const xmlChar*)(VALUE1));
			value1 = strtol((char *)cur_data, &endptr, base);
			xmlFree(cur_data);
			cur_data = get_value_by_name(doc, cur_node, (const xmlChar*)(VALUE2));
			value2 = strtol((char *)cur_data, &endptr, base);
			xmlFree(cur_data);

			if (operation == POLICY_FRR)
			{
			}
			else if( operation == POLICY_TIMER)
			{
				struct _policy_element new_policy;
				new_policy.operation = operation;
				new_policy.up_limit = up_limit;
				new_policy.lower_limit = lower_limit;
				new_policy.value[0] = value1;
				new_policy.value[1] = value2;
				add_policy( g_pt, &new_policy);
			}
			else if ( operation == COMPLEX_DDC_FLAG)
			{
//				ddc_flag = 1;
//				param = value1;
//				throughput = value2;
			}
		}
	}
	return 0;
}

int process_policy(char *xml, int len)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node =NULL;

	doc = xmlParseDoc((xmlChar*)xml);

	if(doc == NULL)
	{
		DEBUG(ERROR, "xmlParseDoc failed %s", strerror(errno));
		return -1;
	}
	root_node = xmlDocGetRootElement(doc);
	if (root_node == NULL) {
		DEBUG(ERROR, "xmlDocGetRootElement failed %s", strerror(errno));
		return -1;
	}

	init_policy_table( g_pt );

	if (parse_policy(doc, root_node) != 0) {
		DEBUG(ERROR, "Can't parse the policy.");
		return -1;
	}

	print_policy_table( g_pt );

	xmlFreeDoc(doc);
	xmlCleanupParser();
	return 0;
}
