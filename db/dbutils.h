#ifndef DBUTILS_H

#define DBUTILS_H

#define DBFILE "pma.db"
#define TABLE_LSDB "pma_lsdb"
#define TABLE_PDB	"pma_pdb"
#define MAX_QUERY_LEN 10240

struct sqlresult{
	int colnum;
	char **colname;
	char **data;
	struct sqlresult * next;
};

struct query_result{
	int total;
	struct sqlresult* result;
	struct sqlresult* tail;
};
	

int db_open();

int db_query();

int db_close();


#endif /* end of include guard: DBUTILS_H */
