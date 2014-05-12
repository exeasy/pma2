#ifndef PMA_CONTROL_H
#define PMA_CONTROL_H

typedef int (*trigger)(void *args);
typedef int ctl_sid;

struct ctl {
	trigger* id;
	int *flag;
	int length;
	struct ctl* next;
};

extern struct ctl * ctls;


int init_ctls();

struct ctl *create_ctl(int size);

int run_ctl(struct ctl* ctlid, int i, void *args );

int add_ctl(struct ctl* ctlid, int i, trigger func, int enabled);


#endif
