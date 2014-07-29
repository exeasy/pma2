#include <utils/common.h>
#include <utils/utils.h>
#include <comm/comm_lite.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include <sys/stat.h>

#define FILE_MODIFY IN_MODIFY




int send_config_to_pma(const char* filename, const char* ip, int port, int type ) { FILE* fp = fopen(filename, "r");
	if ( fp == NULL )
	{
		printf("File no exist!\n");
		return 0;
	}
	fseek(fp,0, SEEK_END);
	unsigned long filesize = ftell(fp);
	fseek(fp,0, SEEK_SET);
	char* buff = (char*)malloc(filesize);
	fread(buff ,1, filesize,fp);
	
	send_message_to_agent( ip, port ,type, buff, filesize);
	free(buff);

}


long int mtime;


int main(int argc, const char *argv[])
{
	if( argc < 5)
	{
		printf("Usage: qconf file_to_detect pmaip pmaport pkt_type\n");
		exit(0);
	}

	FILE* fp = fopen( argv[1] , "r");
	if( fp == NULL ){
		printf("File not existed\n");
		return -1;
	}
	fclose(fp);

	int ir = inotify_init();
	while(1)
	{
	int ret = inotify_add_watch(ir, argv[1], IN_ATTRIB|IN_DELETE_SELF|IN_ONESHOT);
	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(ir, &rset);
	int res = select(ir+1, &rset, NULL, NULL, NULL);
	if( res < 0 ){
		printf("select error\n");
	}
	else if( res == 0){
		printf("select time out'n");
	}
	else {
		char buff[2048];
		read(ir, buff, 2024);

		FILE* fp = fopen( argv[1] , "r");
		if( fp == NULL ){
		printf("File not existed\n");
			continue;;
		}
		fclose(fp);

		struct stat files_info;
		stat(argv[1], &files_info);
		if( mtime != files_info.st_mtime )
		{
		mtime=files_info.st_mtime;//the motified time

		send_config_to_pma(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
		}
	}
	}
	return 0;
}
