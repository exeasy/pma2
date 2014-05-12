#include <utils/common.h>
#include <utils/utils.h>
#include <router/interface.h>
#include<stdio.h>
#include <string.h>


//struct ifTable ifList;
struct ifTable *ifList;
int main()
{
	char routerip[24];
	sprintf(routerip, "192.168.3.1");
	char routerid[24];
	update_interface_from_snmp(routerip);
//    ifList = ifList->next;
//	while(ifList->next)
//	{
//		printf( "%s\n", ifList->ifDescr );
//        printf( "%s\n", ifList->areaId );
//        printf( "%s\n", ifList->ifOperStatus );
//        printf( "%s\n", ifList->ifdex );
//        printf( "%s\n", ifList->ip );
//        printf( "%s\n", ifList->nbrRouteId );
//        printf( "%s\n", ifList->netMask );
//        ifList = ifList->next;
//	}
}
