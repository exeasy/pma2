#include <utils/common.h>
#include <utils/utils.h>
#include <comm/comm_lite.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>


int detect_config(const char* filepath, long int *lastmtime){
	 long int mtime;
     struct stat files_info;
     stat(filepath, &files_info);
     mtime=files_info.st_mtime;//the motified time
	 if( *lastmtime != mtime )
	 {
		 *lastmtime = mtime;
		  return 0;
	 }
	 else return 1;
}

int send_config_to_pma(const char* filename, const char* ip, int port, int type )
{
	FILE* fp = fopen(filename, "r");
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

	long int mtime;
	while(1){
		if ( detect_config(argv[1], &mtime) == 0 ){
			send_config_to_pma(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
		}
	}



	return 0;
}
