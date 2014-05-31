#include <utils/common.h>
#include <utils/xml.h>
#include <comm/comm_lite.h>
#include <sys/stat.h>


#define	QUAGGA_FILE "/etc/quagga/Quagga.conf"
#define BGP_FILE	"/etc/quagga/bgpd.conf"
#define OSPF_FILE	"/etc/quagga/ospfd.conf"

#define OPS_PMA_ROUTER_CONF_INFO	6
#define OPS_PMA_OSPF_CONF_INFO		7
#define OPS_PMA_BGP_CONF_INFO		10
#define OPS_PMS_PMA_INFO_ACK		18
#define OPS_PMS_PMA_INFO_ERR_ACK	19


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

int send_config_to_pma(const char* filepath, int type)
{
	FILE* fp = fopen(filepath, "rw");
	if ( fp == NULL )
	{
		printf("File no exist!\n");
		return 0;
	}
	fseek(fp,0, SEEK_END);
	unsigned long filesize = ftell(fp);
	fseek(fp,0, SEEK_SET);
	char* buff = (char*)malloc(filesize);
	fread(buff,1, filesize,fp);

	send_message_to_agent("127.0.0.1",80,type,buff, filesize);
	free(buff);
}

int conf_detect_daemon(){
	long int mtime1,mtime2,mtime3;
	while(1){
		if( detect_config(QUAGGA_FILE, &mtime1) == 0){
			send_config_to_pma(QUAGGA_FILE,OPS_PMA_ROUTER_CONF_INFO);
		}
		if( detect_config(BGP_FILE, &mtime2) == 0){
			send_config_to_pma(BGP_FILE,OPS_PMA_BGP_CONF_INFO);
		}
		if( detect_config(OSPF_FILE, &mtime3) == 0){
			send_config_to_pma(OSPF_FILE,OPS_PMA_OSPF_CONF_INFO);
		}
		sleep(2);
	}
}

int main(int argc, const char *argv[])
{
	conf_detect_daemon();
	return 0;
}
