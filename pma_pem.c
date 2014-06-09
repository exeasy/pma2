#include <utils/common.h>
#include <utils/utils.h>
#include <server/pem.h>
extern char bm_addr[24];
extern int bm_flag;
int main(int argc, char * argv[])
{
	if( argc > 2)
	{
		bm_flag = 1;
		strcpy(bm_addr, argv[1]);
	}
	pem_init();
	pem_start();
}
