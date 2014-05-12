#include <utils/common.h>
#include <lib/shmem.h>
#include <lib/semph.h>
#include <logger/logger.h>
#include <stdio.h>

int main()
{
	conf_init();
	log_reset();
	log_daemon();
	while(1)
	{
		sleep(10);
	}
}
