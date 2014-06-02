#ifndef ROUTERCMD_H

#define ROUTERCMD_H

#define ROUTER_CMD_PATH "command/sh-cmd/"

struct tunnelcmd {
	__u32 type;
	char src_ip[50];
	char dst_ip[50];
	char next_ip[50];
	char out_if[50];
	__u32 in_label;
	__u32 out_label;
	__u32 outport;
};

int ExecuteRouterCMD(const char* xml, int len);

#endif /* end of include guard: ROUTERCMD_H */
