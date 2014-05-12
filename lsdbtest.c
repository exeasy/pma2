#include "utils/utils.h"
#include "utils/common.h"
#include "lsdb/lsdb.h"

int main(int argc, const char *argv[])
{
	printf("This is a lsdb Test Program\n");
	id_t lsdb;
	cr_lsdb_init(&lsdb, 1);
	int count_of_router, count_of_ls;
	count_of_router = cr_lsdb_get_routers_count(lsdb);
	count_of_ls = cr_lsdb_get_link_states_count(lsdb);

	printf("%d %d\n",count_of_router, count_of_ls);
	return 0;
}
