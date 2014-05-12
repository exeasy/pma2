#include <stdio.h>
#include "includes/snmpcore.h"
#include <includes/snmpget.h>
#include <includes/snmptable.h>
#include <includes/oids.h>

extern tableopt iftable;
extern tableopt ipaddrtable;
extern tableopt iproutetable;

extern tableopt ospfareatable;
extern tableopt ospfiftable;
extern tableopt ospfifmetrictable;
extern tableopt ospfneighbortable;

extern tableopt bgppeertable;
extern tableopt bgprecvpathtable;
extern tableopt bgppathtable;

extern itemopt ospfrouterid;

extern itemopt ospfversion;
extern itemopt bgpversion;
extern itemopt bgplocalas;
extern itemopt bgpidentifier;

int main(int argc, const char *argv[])
{
	session_id_t s = create_session(argv[1],"public");
	
	update_item(s, &ospfrouterid);
	printf("%s\n",ospfrouterid.itemhead);
	clear_item(&ospfrouterid);

	int ver_ospf = 0;
	update_item(s, &ospfversion);
	ver_ospf = atoi(ospfversion.itemhead);
	clear_item(&ospfversion);

	int ver_bgp = 0;
	update_item(s, &bgpversion);
	ver_bgp = atoi(bgpversion.itemhead);
	clear_item(&bgpversion);

	if( ver_ospf == 0 && ver_bgp != 0)
	{
		printf("This is a BGP router\n");
	}else if( ver_ospf == 0 && ver_bgp == 0){
		printf("This is NOT a Router\n");
	}else if( ver_ospf !=0 && ver_bgp == 0){
		printf("This is a OSPF router\n");
	}else{
		printf("This is a Both(ospf & bgp) router\n");
	}
/*
	update_table(s, &iftable);
	show_interface_table(iftable.tablehead);
	clear_table(&iftable);

	update_table(s, &ipaddrtable);
	show_ipaddress_table(ipaddrtable.tablehead);
	clear_table(&ipaddrtable);

	update_table(s, &iproutetable);
	show_iproute_table(iproutetable.tablehead);
	clear_table(&iproutetable);

	update_table(s, &ospfareatable);
	show_ospfarea_table(ospfareatable.tablehead);
	clear_table(&ospfareatable);

	update_table(s, &ospfiftable);
	show_ospfif_table(ospfiftable.tablehead);
	clear_table(&ospfiftable);

	update_table(s, &ospfifmetrictable);
	show_ospfifmetric_table(ospfifmetrictable.tablehead);
	clear_table(&ospfifmetrictable);

	update_table(s, &ospfneighbortable);
	show_ospfneighbor_table(ospfneighbortable.tablehead);
	clear_table(&ospfneighbortable);

	update_table(s, &bgppeertable);
	show_bgppeer_table(bgppeertable.tablehead);
	clear_table(&bgppeertable);
	//update_table(s, &bgprecvpathtable);//not used
	//show_bgppeer_table(bgprecvpathtable.tablehead);
	update_table(s, &bgppathtable);
	show_bgppeer_table(bgppathtable.tablehead);
	clear_table(&bgppathtable);
	printf("Ended\n");
	*/
	return 0;
}
