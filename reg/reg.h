#ifndef PMA_REG_H
#define PMA_REG_H

#define agent_reg() agent_register(0, NULL)
#define agent_unreg() agent_unregister(0, NULL)
/*!
 * * 代理当前的状态
 * */
#define AGENT_LOGIN 1
#define AGENT_LOGOUT 0

extern int agent_register(u32 agent_id, const char *seq);
extern int agent_unregister(u32 agent_id, const char *seq);

extern u8 agent_login_status;

#endif
