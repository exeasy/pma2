#ifndef POLICY_H
#define POLICY_H

#define FRR 1
#define TIMER_ADJUST 2
#define COMPLEX_DDC_FLAG     3

//extern int ddc_flag;
//extern unsigned long long param;
//extern unsigned long long throughput;
//
//extern unsigned long long g_ddc_param;
//extern unsigned long long g_ddc_throughput;
//extern struct timeval get_ddc_param_last_time;
//
//extern int update_register_tag;
//
//typedef struct op_para		//operation's parameter
//{
//	u64 para_value;
//	struct op_para * next_para;
//} op_para;
//
//typedef struct policy		//policy
//{
//	u8 operation_code;
//	struct op_para * para_list;
//
//} policy;
//
//struct _policy_ddc_body
//{
//	u8 policy_type;  /*< 策略的类型*/
//	u8 pep_version;  /*< 策略请求者的软件版本号 */
//	u8 operation;    /*< 表示使用策略为何操作，目前1表示修改定时器的值，2表示采用FRR保护*/
//	u8 blank;        /*< 保留位*/
//	u64 operation_value[2]; /* value[1]=调节因子,value2=吞吐率 */
//};
//
//

int policy_init();
#endif
