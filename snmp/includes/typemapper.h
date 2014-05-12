#ifndef TYPEMAPPER_H

#define TYPEMAPPER_H

//Mapper of Table: iftable
const int iftable_mapper[22] = 
{
	0,/* ifindex */
	1,/* ifdescr */
	2,/* iftype */
	3,/* ifmtu */
	4,/* ifspeed*/
	5,/* ifphysaddress */
	6,/* ifadminstatus */
	7,/* ifoperstatus */
	9,/* ifinoctets */
	15,/* ifoutoctets */
};
//Mapper of Table: ipaddrtable
const int ipaddr_mapper[4] = 
{
	0,/* ipaddr */
	1,/* ifindex */
	2,/* ifmask */
	3,/* bcastaddr */
};
//Mapper of Table: iproutetable
const int iproute_mapper[] = 
{
	0,/* route dest */
	1,/* route mask */
	2,/* route tos  */
	3,/* next hop   */
	4,/* ifindex    */
	5,/* type       */
	6,/* proto		*/
	9,/* next as	*/
	10,/* metric1	*/
	15,/* status	*/
};
//Mapper of Table: ospfAreaTable
const int ospfarea_mapper[] =
{
	0,/* area id */
	3,/* spf count*/
	6,/* lsa count*/
	9,/* status   */
};
//Mapper of Table: ospfIfTable
const int ospfif_mapper[] = 
{
	0,/* if ip address */
	2,/* area id		*/
	3,/* type			*/
	4,/* adminstatus	*/
	6,/* transit timeval*/
	7,/*retran timeval  */
	8,/* hello timeval  */
	9,/* dead timeval  */
	10,/* pool timeval */
	11,/* state		*/
	12,/* dr	*/
	13,/* bdr	*/
	14,/* ifevent */
	16,/* status */
};
//Mapper of Table: ospfIfMetricTable
const int ospfifmetric_mapper[] = 
{
	0,/* if address */
	3,/* metric value*/
	4,/* metric status*/
};
//Mapper of Table: ospfNeighborTable
const int ospfneighbor_mapper[] = 
{
	0,/* ip address */
	2,/* nbr router id*/
	5,/* nbr state  */
	6,/* nbr event count */
	8,/* status */
};
//Mapper of Table: bgpPeerTable
const int bgppeer_mapper[] = 
{
	0,/* identifier */
	1,/* peer state */
	2,/* admin status */
	4,/* local addr */
	5,/* local port */
	6,/* peer addr */
	7,/* peer port */
	8,/* remote as */
	9,/* in updates */
	10,/* out updates*/
	11,/* in messages */
	12,/* out messages */
	15,/* estab time */
	16,/* con retry time */
	17,/* hold time */
	18,/* keepalive */
	19,/* hold time confed */
	20,/* keep time confed */
	21,/* min as origination timeval */
	22,/* min route advertisement inteval */
	23/* update elapsed time */
};

const int bgprecvpath_mapper[] = {
	0,
	1,
	2,
	3,
	4,
	5
};

const int bgppath_mapper[] = {
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12
};

#endif /* end of include guard: TYPEMAPPER_H */
