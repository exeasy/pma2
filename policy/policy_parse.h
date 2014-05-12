#ifndef POLICY_PARSE_H
#define POLICY_PARSE_H


#define VERSION		"Version"
#define OPERATION	"operation"
#define UP_LIMIT	"up_limit"
#define LOWER_LIMIT	"lower_limit"
#define VALUE1		"value1"
#define VALUE2		"value2"
#define POLICY_FRR	FRR
#define POLICY_TIMER	TIMER_ADJUST
//
//struct policy_head{
//	struct list_head list;
//	int version;
//	int operation;
//	unsigned int up_limit;
//	unsigned int lower_limit;
//};
//
//struct policy_frr {
//		struct policy_head ph;
//};
//
//struct policy_timer {
//		struct policy_head ph;
//			unsigned long long value[2];
//};

int process_policy(char * xml,int len);

#endif
