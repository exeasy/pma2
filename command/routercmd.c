#include "routercmd.h"
#include <utils/common.h>
#include <utils/utils.h>
#include <utils/xml.h>

/**
 * 执行cmd命令
 */
int runRouterCmd(char *cmd, int type) {
	int ret = 0;
	//	if (type == 4) {
	//		ret = system(cmd);
	//	} else {
	//		int pid = vfork();
	//		if (pid < 0) {
	//			perror("vfork failed!");
	//		} else if (pid == 0) {
	//			ret = execl("/bin/sh", "sh", "-c", cmd, (char *) 0);
	//
	//		} else {
	//			addpid(pid);
	//		}
	//	}
	ret = system(cmd);
	printf("cmd seq =%d\n", ret);
	if (ret != 0) {
		DEBUG(INFO,"cmd run error");
	}

	return 0;
}
int ExecuteRouterCMD(const char* xml, int len)
{
	if (xml == NULL || len <= 0) {
		return -1;
	}
	struct {
		char src_ip[50];
		char dst_ip[50];
		char next_ip[50];
		char out_if[50];
		__u32 in_label;
		__u32 out_label;
		int outport;
	} cmd_arg;
	xmlDocPtr doc;
	xmlNodePtr node;
	xmlNodePtr contextnode;
	xmlNodePtr flownode;
	char *data = NULL;
	char routerCmd[200];
	char checkCmd[200];
	char ip[50];
	int cmd_type;
	int ret = 1;
	//	int cmd_len = 0;
	DEBUG(INFO,"recive cmd");
	memset(&cmd_arg, 0, sizeof(cmd_arg));
	doc = xmlParseMemory(xml, len);
	if (doc == NULL) {
		perror("config doc is null!");
		return (-1);
	}
	memset(routerCmd, '\0', sizeof(routerCmd));
	memset(ip, '\0', sizeof(ip));
	//添加脚本路径
	sprintf(routerCmd, "%s\0", ROUTER_CMD_PATH);
	len = strlen(routerCmd);
//	addr_to_str(get_router_addr(), ip, sizeof(ip) - 1);
	//get node info
	node = xmlDocGetRootElement(doc);
	for (node = node->children; node != NULL; node = node->next) {
		/* skip entity refs */
		data = (char *) xmlNodeGetContent(node);
		if (strcmp((char *) node->name, "type") == 0) {
			sscanf(data, "%x", &cmd_type);
			//添加脚本名
			sprintf(routerCmd + len, "%.2x", cmd_type);
			len = strlen(routerCmd);
			printf("type = %s\n", data);
		}
		if (strcmp((char *) node->name, "content") == 0) {
			contextnode = node->children;
			for (; contextnode != NULL; contextnode = contextnode->next) {
				if (strcmp((char *) contextnode->name, "flow") == 0) {
					flownode = contextnode;
					for (flownode = flownode->children; flownode != NULL; flownode
							= flownode->next) {
						if (strcmp((char *) flownode->name, "text") == 0
								|| strcmp((char *) flownode->name, "comment")
										== 0) {
							continue;
						}
						data = (char *) xmlNodeGetContent(flownode);
						if (strcmp((char *) flownode->name, "src_ip") == 0) {
							sprintf(routerCmd + len, " %s", data);
							len = strlen(routerCmd);
							sprintf(cmd_arg.src_ip, "%s", data);
						} else if (strcmp((char *) flownode->name, "dst_ip")
								== 0) {
							sprintf(routerCmd + len, " %s", data);
							len = strlen(routerCmd);
							sprintf(cmd_arg.dst_ip, "%s", data);
						} else if (strcmp((char *) flownode->name, "out_if")
								== 0) {
							sprintf(routerCmd + len, " %s", data);
							len = strlen(routerCmd);
							sprintf(cmd_arg.out_if, "%s", data);
						} else if (strcmp((char *) flownode->name, "next_ip")
								== 0) {
							sprintf(routerCmd + len, " %s", data);
							len = strlen(routerCmd);
							sprintf(cmd_arg.next_ip, "%s", data);
						} else if (strcmp((char *) flownode->name, "in_label")
								== 0) {
							sprintf(routerCmd + len, " %s", data);
							len = strlen(routerCmd);
							sscanf(data, "%d", &cmd_arg.in_label);
						} else if (strcmp((char *) flownode->name, "out_label")
								== 0) {
							sprintf(routerCmd + len, " %s", data);
							len = strlen(routerCmd);
							sscanf(data, "%d", &cmd_arg.out_label);
						}
						//添加参数

					}
				} else if (strcmp((char *) contextnode->name, "outport") == 0) {
					flownode = contextnode;
					data = (char *) xmlNodeGetContent(flownode);
					sprintf(routerCmd + len, " %s", data);
					len = strlen(routerCmd);
					sscanf(data, "%d", &cmd_arg.outport);
				}
			}
			xmlFree(data);
		}
	}

	//添加路由器ip
	//	sprintf(routerCmd + len, " %s", ip);
	//命令类型大于0x30为BGP相关命令，其他为IGP相关命令
	if (cmd_type > 0x30) {
		sprintf(routerCmd + len, " %s", ip);
	}
	len = strlen(routerCmd);
	puts(routerCmd);
	DEBUG(INFO,"%s",routerCmd);

	u32 rkey;
	u32 key;
	u32 num;
	//执行命令
	/*
	if (cmd_type >= 1 && cmd_type <= 3) {
		switch (cmd_type) {
		case 1:
			DEBUG(INFO,"run cmd");
			add_mpls_in(cmd_arg.out_label, cmd_arg.out_if, cmd_arg.next_ip,
					&rkey);
			get_iptables(strcmp(cmd_arg.src_ip, "") ? cmd_arg.src_ip : NULL,
					cmd_arg.dst_ip, &num, &key);
			if (num != 0) {
				del_iptables(num);
				//				del_mpls(key);
			}
			add_iptables(strcmp(cmd_arg.src_ip, "") ? cmd_arg.src_ip : NULL,
					cmd_arg.dst_ip, rkey);
			LOG("end cmd");
			break;
		case 2:
			LOG("run cmd");
			add_mpls_end(cmd_arg.in_label);
			add_mpls_in(cmd_arg.out_label, cmd_arg.out_if, cmd_arg.next_ip,
					&rkey);
			add_mpls_mid(cmd_arg.in_label, rkey);
			LOG("end cmd");
			break;
		case 3:
			LOG("run cmd");
			add_mpls_end(cmd_arg.in_label);
			LOG("end cmd");
			break;
		}
	} else {
		LOG("run cmd");
		runRouterCmd(routerCmd, cmd_type);
		LOG("end cmd");
	}
	*/
	DEBUG(INFO,"run cmd");
	runRouterCmd(routerCmd, cmd_type);
	DEBUG(INFO,"end cmd");

	xmlFree(data);
	xmlFreeDoc(doc);
	return 0;
}
