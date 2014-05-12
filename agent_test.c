#include <stdio.h>
#include <utils/utils.h>
#include <utils/common.h>
#include <comm/comm_utils.h>
#include <comm/header.h>

int main()
{
	char *ip1 = "192.168.84.1";
	char *id1 = "0.0.0.1";
	char *ip2= "192.168.84.128";
	char *id2 = "0.0.0.2";
	struct in_addr temp1,temp2,temp3,temp4;
	inet_pton(AF_INET, ip1, &temp1);
	inet_pton(AF_INET, id1, &temp2);
	inet_pton(AF_INET, ip2, &temp3);
	inet_pton(AF_INET, id2, &temp4);
	int length = sizeof(struct pma_pms_header) + 16;
	struct pma_pms_header *pkt = (struct pma_pms_header*)malloc(length);
	pkt->pph_pkg_dev_type = 0;
	pkt->pph_pkg_type = NEIGHBOR_LIST;
	pkt->pph_pkg_version = PMA_VERSION;
	pkt->pph_pkg_len = htonl(length);
	pkt->pph_agent_id = temp2.s_addr;
	char buff[1024];
	sprintf(buff, "
	
}
