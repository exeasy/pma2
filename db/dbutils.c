#include <stdio.h>
#include <utils/utils.h>
#include <utils/common.h>
#include <sqlite3.h>
#include "dbutils.h"

sqlite3 *db;


int free_result(struct query_result * query_result)
{
	struct sqlresult * temp = query_result->result;
	while(temp)
	{
		struct sqlresult * f = temp;
		temp = temp->next;
		free(f);
	}
	free(query_result);
	query_result = NULL;
}

static int callback_save_result(void *callresult, int argc, char **argv, char **azColName){
	struct query_result * query_result = (struct query_result*) callresult;
	query_result->total++;
	struct sqlresult * result  = malloc_z(struct sqlresult);
	
	if( query_result->tail == NULL)
	{
		query_result->result = result;
		query_result->tail = result;
	}
	else
	{
		query_result->tail->next = result;
		query_result->tail = result;
	}
	
	result->colnum = argc;
	result->colname = (char**)malloc(sizeof(char*)*argc);
	result->data = (char**)malloc(sizeof(char*)*argc);
	int i = 0;
	for( ; i < argc ; i++)
	{
		int len_col = strlen(azColName[i]);
		int len_dat = strlen(argv[i]);
		result->colname[i] = (char*)malloc(len_col+1);
		result->data[i] = (char*)malloc(len_dat+1);
		strcpy(result->colname[i], azColName[i]);
		strcpy(result->data[i], argv[i]);
	}
	return 0;
}

int show_result(struct query_result * result)
{
	struct  sqlresult * temp = result->result;
	while(temp)
	{
		struct sqlresult * h = temp;
  int i;
  for(i=0; i< h->colnum; i++){
    printf("%s = %s\n", h->colname[i], h->data[i] ? h->data[i] : "NULL");
  }
  printf("\n");
  temp = temp->next;
	}
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}


int db_open()
{
  int rc = sqlite3_open(DBFILE, &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(-1);
  }
  else return (rc);
}

int db_query_call(const char* query_cmd, struct query_result *query_result)
{
  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, query_cmd , callback_save_result, query_result, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return (0);
}
int db_query(const char* query_cmd)
{
  char *zErrMsg = 0;
  int rc = sqlite3_exec(db, query_cmd , callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  return (0);
}

int db_close()
{
  sqlite3_close(db);
}

