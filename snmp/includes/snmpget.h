/*
 * netsnmp.h
 *
 *  Created on: Jun 4, 2012
 *      Author: yx
 */

#ifndef NETSNMP_H_
#define NETSNMP_H_

int snmpget(char * address, char * community, char * oidNO ,char * rs);

struct ptrmanager{
	void * ptr;
	struct ptrmanager *next;
};

#define INIT_CLEANER struct ptrmanager * ptr_head = NULL;
#define FREE_CLEANER \
	do{ \
		struct ptrmanager * ptr = ptr_head; \
		while(ptr){\
			if(ptr->ptr)\
			{\
			free(ptr->ptr);\
			}\
			free(ptr);\
			ptr_head = ptr_head->next;\
			ptr = ptr_head;\
		}\
	}while(0);

#define Z_strdup(x) \
	({\
	struct ptrmanager * kptr = (struct ptrmanager*)malloc(sizeof(struct ptrmanager));\
		kptr->ptr = strdup(x);\
		kptr->next = NULL;\
		if (ptr_head == NULL)\
		ptr_head = kptr;\
		else \
		{\
			kptr->next = ptr_head;\
			ptr_head = kptr;\
		}\
		kptr->ptr;\
	})

#endif /* NETSNMP_H_ */
