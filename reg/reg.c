#include <utils/common.h>
#include <utils/utils.h>
#include <comm/header.h>
#include <comm/comm.h>
#include <reg.h>

u8 agent_login_status = AGENT_LOGOUT;

static void *login(void *args);

int agent_register(u32 agent_id, const char *seq)
{
	struct req_args *login_args =
		(struct req_args *)malloc(sizeof(struct req_args));

	memset(login_args->ip, 0, sizeof(login_args->ip));
	char *ip = get_server_address();
	memcpy(login_args->ip, ip, strlen(ip));
	login_args->port = get_server_port();
	login_args->ops_type = OPS_PMA_LOGIN;
	login_args->len = 0;

	if (login(login_args) == ((void *)-1)) {
		free(login_args);
		return -1;
	}
	free(login_args);
	agent_login_status = AGENT_LOGIN;
	return SUCCESS;
}

static void * login(void *args)
{
	struct req_args *login_args =
		(struct req_args *)args;

	int ret = send_message_to_pms(login_args->ip,
			login_args->port,
			login_args->ops_type,
			NULL,
			0);
	if (ret == -1) {
		return PTR_ERR;
	} else {
		return NULL;
	}
}

int agent_unregister(u32 agent_id, const char *seq)
{
	return SUCCESS;
}

