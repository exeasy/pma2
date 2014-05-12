#ifndef LSD_HANDLER_H

#define LSD_HANDLER_H

typedef union state_changer
{
	u_int8_t data;
	struct
	{
		u_int8_t hello_low : 1;
		u_int8_t hello_high : 1;
		u_int8_t info6 : 1;
		u_int8_t info5 : 1;
		u_int8_t info4 : 1;
		u_int8_t info3 : 1;
		u_int8_t info2 : 1;
		u_int8_t info1 : 1;

	}field;
}state_changer;

void bbe_up_handler(struct backbone_eth* eth);
void ace_up_handler(struct access_eth* eth);

void hello_changed_handler(struct backbone_eth* eth,
		enum priority_type type, enum lsd_status status);
void hello_exchange_handler(struct backbone_eth* eth,
		enum priority_type type, enum lsd_status status);
void on_exchange_finished(struct backbone_eth* eth);


#endif /* end of include guard: LSD_HANDLER_H */
