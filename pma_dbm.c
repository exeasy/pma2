#include <utils/utils.h>
#include <utils/common.h>
#include <server/dbm.h>

int main(int argc, const char *argv[])
{
	dbm_init();
	dbm_start();
	return 0;
}
