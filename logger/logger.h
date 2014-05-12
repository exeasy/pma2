#ifndef LOGGER_H_
#define LOGGER_H_

#define MAX_EVENT_NUM 1024
#define MAX_EVENT_NAME_LEN 20
#define MAX_BUFF_LEN 10240
#define MAX_NAME_LEN 50

#define BLOCK_SIZE 4096
#define MAX_BLOCK 20

#define EMPTY	1
#define FULL	2
#define MUTEX	0

#define CONFILE "pmalog.conf"
// Logger Usage:
// For Server
// conf_init();
// log_reset();
// log_daemon();
//
// For Client
// log_type_init();
// log_init();
// Then use the logger(EVENTNAME, format , ...)
// to send the message to logserver
//
struct logtype{
	char name[MAX_NAME_LEN];
	unsigned int index;
	struct timeval record_time;
	struct timeval last_time;
	int flag;
	struct logtype *next;
};

struct logmsg{
	char name[MAX_NAME_LEN];
	struct timeval record_time;
	struct timeval last_time;
	unsigned int type;
	unsigned int length;
	unsigned char data[0];
};


typedef int (*functpr)(void*);

int fork_daemon(void);

int log_type_init();

int log_type_add(const char* typename, int enabled);

struct logtype *get_logtype_addr(const char* name);

int get_typeinfo(struct logmsg* msg);

int log_init();

int log_reset();

int log_daemon();

int logger(const char* name, const char* format, ...);

int log_writer(struct logmsg *msg);

char* log_reader();

int log_close();

char* log_package_xml(struct logmsg* msg);

char* log_package_json(struct logmsg* msg);
#endif
