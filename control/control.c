#include <utils/common.h>

#include "control.h"

struct ctl * ctls;

int init_ctls()
{
	ctls = (struct ctl*)malloc(sizeof(struct ctl));
	if(ctls == NULL) return -1;
	ctls->next = NULL;
	ctls->length = 0;
	ctls->id = 0;
	return 0;
}

struct ctl* create_ctl(int size)
{
	trigger* sid = (trigger*)calloc(size, sizeof(trigger));
	int *flag = (int*)calloc(size, sizeof(int));
	if(sid == 0) return -1;
	struct ctl* nctl = (struct ctl*)malloc(sizeof(struct ctl));
	nctl->next = ctls->next;
	nctl->id = sid;
	nctl->flag = flag;
	nctl->length = size;
	ctls->next = nctl;
	return nctl;
}

int run_ctl(struct ctl* ctlid, int i, void *args )
{
	//struct ctl* ctlid = (struct ctl*)id;
	if( i <0 || i > ctlid->length)
		return -1;
	trigger* list = ctlid->id;

	if(ctlid->flag[i]== 0) 
		 return -1;
	int ret = list[i](args);
	return ret;
}

int add_ctl(struct ctl* ctlid, int i, trigger func,int enabled)
{
	//struct ctl* ctlid = (struct ctl*)id;
	if( i< 0 || i > ctlid->length)
		return -1;
	trigger* list = (trigger*)ctlid->id;
	if( list[i] != func)
	list[i] = func;
	ctlid->flag[i] = enabled;
	return 0;
}
