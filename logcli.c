#include <utils/common.h>
#include <logger/logger.h>
#include <stdio.h>
int main()
{
	log_type_init();
//	log_type_add("pmaregister",1);
//	log_type_add("pmadaemon",1);
	log_init();

	while(1)
	{
	logger("PMA_REGISTER","pma %d register",1);
	logger("PMA_REGISTER_OK","pma %d register ack received",1);
	logger("LINK_UP","pma daemon started",1);
	sleep(1);
	}
}
